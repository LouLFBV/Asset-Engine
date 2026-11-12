#include "AssetManager.h"
#include "Checksum.h"
#include "Log.h"

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
}

//ASYNCHRONE (Ne bloque pas le jeu)
void AssetManager::LoadAsync(Asset* asset) {
    if (asset == nullptr || asset->objet != nullptr) return;


    // 1. Sécurité : Vérifier qu'on n'est pas déjà en train de le charger
    // (Pour éviter de lancer 10 threads si le joueur clique 10 fois)
    if (m_loadingTasks.find(asset->uiid) != m_loadingTasks.end()) {
        return;
    }

    // 2. On lance le thread ET ON STOCKE LE RETOUR dans la map
    // std::launch::async force la création d'un vrai thread séparé
     m_assetJobSystem.Submit(m_loadingTasks[asset->uiid]);
}

// MISE A JOUR (A appeler à chaque frame dans ton jeu)
void AssetManager::Update() {
    // On parcourt les chargements en cours pour nettoyer ceux qui ont fini
    for (auto it = m_loadingTasks.begin(); it != m_loadingTasks.end(); ) {

        // On demande l'état du thread avec un délai d'attente de 0 seconde (donc ça ne bloque pas)
        if (it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            // Le chargement est terminé ! On retire la tâche de la liste.
            it = m_loadingTasks.erase(it);
        }
        else {
            // Toujours en cours de chargement, on passe au suivant.
            ++it;
        }
    }
}