#include "AssetManager.h"

#include "Checksum.h"
#include "Log.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "BinaryArchive.h"

#include <filesystem>
#include <memory>
#include <algorithm>
#include <mutex> 

AssetManager::AssetManager() = default;

AssetManager::~AssetManager()
{
    // Attendre tous les chargements avant de detruire les assets.
    m_assetJobSystem.WaitIdle();

    ClearRegistry();
}

void AssetManager::ClearRegistry()
{
    // Protection thread-safe globale du registre si besoin, ou locale
    for (auto& [id, asset] : m_assetRegistry)
    {
        if (asset == nullptr)
        {
            continue;
        }

        // Destruction de l'objet charge.
        {
            std::lock_guard<std::mutex> lock(asset->mutex);

            delete asset->objet;
            asset->objet = nullptr;
        }

        delete asset;
    }

    m_assetRegistry.clear();

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.clear();
    }
}


void AssetManager::LoadSync(Asset* asset)
{
    if (asset == nullptr)
        return;

    // 1. Vérifier rapidement si l'asset est déjà chargé.
    {
        std::lock_guard<std::mutex> lock(asset->mutex);

        if (asset->objet != nullptr)
            return;
    }

    try
    {
        // 2. Vérification de l'extension.
        bool isJson = asset->path.ends_with(".json");
        bool isBinary = asset->path.ends_with(".bin");

        if (!isJson && !isBinary)
        {
            LOG_ERROR("Format d'asset non supporte.");
            throw std::runtime_error("Format non supporte.");
        }

        // 3. Vérification du checksum.
        uint32_t currentChecksum =
            Checksum::ComputeFromFile(asset->path);

        if (currentChecksum == 0 ||
            (asset->checksum != 0 &&
                asset->checksum != currentChecksum))
        {
            LOG_ERROR("Checksum invalide ou fichier inaccessible.");
            throw std::runtime_error("Checksum invalide.");
        }

        LOG_INFO("Checksum valide.");

        // 4. Création de l'objet temporaire.
        std::unique_ptr<RTTI> loadedObject(
            Factory::Create(asset->type)
        );

        if (!loadedObject)
        {
            LOG_ERROR("Impossible de creer l'objet via la Factory.");
            throw std::runtime_error("Factory::Create a echoue.");
        }

        // 5. Désérialisation SANS verrouiller le mutex.
        if (isJson)
        {
            JsonArchive archive(
                asset->path,
                ArchiveDirection::Read
            );

            loadedObject->Process(archive);
        }
        else
        {
            BinaryArchive archive(
                asset->path,
                ArchiveDirection::Read
            );

            loadedObject->Process(archive);
        }

        // 6. Publication de l'objet.
        {
            std::lock_guard<std::mutex> lock(asset->mutex);

            if (asset->objet == nullptr)
            {
                asset->objet = loadedObject.release();
            }
        }

        LOG_INFO(
            ("Asset charge : " + asset->path).c_str()
        );
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(
            ("Erreur de chargement : " +
                std::string(e.what())).c_str()
        );
    }
    catch (...)
    {
        LOG_ERROR("Erreur inconnue durant le chargement.");
    }

    // 7. Signaler la fin du chargement.
    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);
    }
}


void AssetManager::LoadAsync(Asset* asset)
{
    if (asset == nullptr || asset->IsLoaded())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);

        // Eviter plusieurs chargements du meme asset.
        if (!m_loadingTasks.emplace(asset->uiid).second)
        {
            return;
        }
    }

    try
    {
        m_assetJobSystem.Submit([this, asset]()
            {
                LoadSync(asset);
            });
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);

        throw;
    }
}

void AssetManager::ScanDirectory(const std::string& directoryPath, std::vector<Pawn*>& worldPawns)
{
    if (!std::filesystem::exists(directoryPath)) {
        LOG_ERROR("Le dossier d'assets n'existe pas : " + directoryPath);
        return;
    }

    // Attendre que les tâches en cours soient terminées pour éviter les conflits d'écriture/lecture sur le registre
    m_assetJobSystem.WaitIdle();

    // 1. Collecter tous les chemins valides trouvés sur le disque
    std::vector<std::string> foundFiles;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".json" || ext == ".bin") {
                std::string filePath = entry.path().string();
                std::filesystem::path p(filePath);
                std::string stem = p.stem().string(); // ex: "pawn_7"

                // --- FILTRE ANTI-DOUBLON (JSON prioritaire sur BIN) ---
                // Si on trouve un .bin, mais qu'on a déjà vu ou qu'on possède le .json correspondant, on ignore le .bin
                bool skip = false;
                for (const auto& existingPath : foundFiles) {
                    std::filesystem::path existingP(existingPath);
                    if (existingP.stem().string() == stem) {
                        // Si le fichier actuel est un .bin et que l'autre est un .json, on ignore le .bin
                        if (ext == ".bin" && existingP.extension().string() == ".json") {
                            skip = true;
                            break;
                        }
                        // Inversement, si le fichier actuel est un .json et que l'autre est un .bin, 
                        // on peut remplacer le .bin par le .json dans la liste
                        if (ext == ".json" && existingP.extension().string() == ".bin") {
                            // Optionnel : on pourrait nettoyer le .bin du disque, mais pour l'instant on l'ignore juste dans l'index
                            skip = true;
                            break;
                        }
                    }
                }

                if (skip) continue;
                // -----------------------------------------------------

                foundFiles.push_back(filePath);
                uint32_t diskChecksum = Checksum::ComputeFromFile(filePath);

                // Chercher si l'asset existe déjà dans le registre
                Asset* existingAsset = nullptr;
                for (auto& [id, assetPtr] : m_assetRegistry) {
                    if (assetPtr != nullptr && assetPtr->path == filePath) {
                        existingAsset = assetPtr;
                        break;
                    }
                }

                if (existingAsset != nullptr) {
                    if (existingAsset->checksum != diskChecksum) {
                        LOG_WARN(("ALERTE : L'asset a ete modifie sur le disque ! -> " + filePath).c_str());
                        existingAsset->checksum = diskChecksum;
                    }
                }
                else {
                    // Nouvel asset découvert
                    Asset* newAsset = new Asset();
                    newAsset->uiid = Guid::Generate();
                    newAsset->path = filePath;
                    newAsset->type = "Pawn";
                    newAsset->objet = nullptr;
                    newAsset->checksum = diskChecksum;

                    m_assetRegistry[newAsset->uiid] = newAsset;
                    LOG_INFO("Nouvel asset indexe dans le Registry : " + filePath);
                }
            }
        }
    }

    // 2. NETTOYAGE DU REGISTRE ET DES PAWNS DU MONDE
    for (auto it = m_assetRegistry.begin(); it != m_assetRegistry.end(); ) {
        Asset* asset = it->second;
        if (asset != nullptr) {
            auto foundIt = std::find(foundFiles.begin(), foundFiles.end(), asset->path);
            if (foundIt == foundFiles.end()) {
                LOG_INFO(("Suppression du registre (fichier absent du disque) : " + asset->path).c_str());

                // Nettoyer les pawns du monde (`worldPawns`) qui pointaient vers cet asset supprimé
                if (asset->objet != nullptr) {
                    Pawn* deadPawn = SAFE_CAST(Pawn, asset->objet);
                    if (deadPawn) {
                        // Retirer le pawn du vecteur worldPawns pour éviter les pointeurs pendouillants
                        worldPawns.erase(
                            std::remove(worldPawns.begin(), worldPawns.end(), deadPawn),
                            worldPawns.end()
                        );
                    }

                    delete asset->objet;
                    asset->objet = nullptr;
                }

                delete asset;
                it = m_assetRegistry.erase(it);
                continue;
            }
        }
        ++it;
    }
}

JobSystem& AssetManager::GetJobSystem()
{
    return m_assetJobSystem;
}