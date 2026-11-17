#include "AssetManager.h"
#include "Checksum.h"
#include "Log.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "BinaryArchive.h" // N'oublie pas d'inclure le binaire aussi !
#include <filesystem>

AssetManager::AssetManager() = default;

void AssetManager::LoadSync(Asset* asset) // On prend un pointeur Asset* pour modifier l'original en mémoire
{
    if (asset == nullptr || asset->objet != nullptr) return;

    // 1. Vérification par Checksum de l'intégrité du fichier avant le chargement
    uint32_t currentChecksum = Checksum::ComputeFromFile(asset->path);
    if (asset->checksum != 0 && asset->checksum != currentChecksum) {
        LOG_ERROR("Le fichier est errone ! Il n'est pas charge !");
        m_loadingTasks.erase(asset->uiid);
        return;
    }
    else {
        LOG_INFO("Le checksum est valide !");
    }

    // 2. Instanciation via la Factory
    asset->objet = Factory::Create(asset->type);

    if (asset->objet != nullptr) {
        // 3. Choix dynamique de l'archive selon l'extension (.json ou .bin)
        if (asset->path.ends_with(".json")) {
            JsonArchive archive(asset->path, ArchiveDirection::Read);
            asset->objet->Process(archive);
        }
        else if (asset->path.ends_with(".bin")) {
            BinaryArchive archive(asset->path, ArchiveDirection::Read);
            asset->objet->Process(archive);
        }
    }

    // Nettoyage de la tâche en cours
    m_loadingTasks.erase(asset->uiid);
}

// ASYNCHRONE (Ne bloque pas le jeu)
void AssetManager::LoadAsync(Asset* asset) // Passage en pointeur ici aussi
{
    if (asset == nullptr || asset->objet != nullptr) return;

    // Sécurité : Vérifier qu'on n'est pas déjà en train de le charger
    if (m_loadingTasks.find(asset->uiid) != m_loadingTasks.end()) {
        return;
    }
    m_loadingTasks.emplace(asset->uiid);

    // On délègue au JobSystem pour exécuter LoadSync en arrière-plan sur un autre thread
    m_assetJobSystem.Submit([this, asset]() {
        LoadSync(asset);
        });
}

void AssetManager::ScanDirectory(const std::string& directoryPath)
{
    // Optionnel : Nettoyer l'ancienne map avant de rescanner pour éviter les fuites
    for (auto& [id, assetPtr] : m_assetRegistry) {
        delete assetPtr;
    }
    m_assetRegistry.clear();

    if (!std::filesystem::exists(directoryPath)) {
        LOG_ERROR("Le dossier d'assets n'existe pas : " + directoryPath);
        return;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".json" || ext == ".bin") {
                // ALLOCATION DYNAMIQUE OBLIGATOIRE POUR ÉVITER LE CRASH DE POINTEUR
                Asset* newAsset = new Asset();
                newAsset->uiid = Guid::Generate();
                newAsset->path = entry.path().string();
                newAsset->type = "Pawn";
                newAsset->objet = nullptr;
                newAsset->checksum = Checksum::ComputeFromFile(newAsset->path);

                m_assetRegistry[newAsset->uiid] = newAsset;
                LOG_INFO("Asset indexe dans le Registry : " + newAsset->path);
            }
        }
    }
}

JobSystem& AssetManager::GetJobSystem()
{
    return m_assetJobSystem;
}