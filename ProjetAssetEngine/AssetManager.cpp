#include "AssetManager.h"
#include "Checksum.h"
#include "Log.h"

AssetManager::AssetManager() = default;


void AssetManager::LoadSync(Asset* asset) {
    if (asset == nullptr || asset->objet != nullptr) return;

    if (asset->checksum != Checksum::ComputeFromFile(asset->path)) {
        LOG_ERROR("Le fichier est errone ! Il n'est pas charge !");
        return;
    }
    else {
        LOG_INFO("Le checksum est valide !");
    }

    asset->objet = Factory::Create(asset->type);

    if (asset->objet != nullptr) {
        JsonArchive archive(asset->path, ArchiveDirection::Read);
        asset->objet->Process(archive);
    }
    m_loadingTasks.erase(asset->uiid);
}

//ASYNCHRONE (Ne bloque pas le jeu)
void AssetManager::LoadAsync(Asset* asset) {
    if (asset == nullptr || asset->objet != nullptr) return;


    // 1. Sécurité : Vérifier qu'on n'est pas déjà en train de le charger
    // (Pour éviter de lancer 10 threads si le joueur clique 10 fois)
    if (m_loadingTasks.find(asset->uiid) != m_loadingTasks.end()) {
        return;
    }
    m_loadingTasks.emplace(asset->uiid);

    m_assetJobSystem.Submit([this, asset]() {LoadSync(asset); });
}

JobSystem& AssetManager::GetJobSystem()
{
    return m_assetJobSystem;
}