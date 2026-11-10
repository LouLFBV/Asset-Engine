#pragma once
#include <unordered_map>
#include <future>
#include <chrono>
#include "Asset.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "RTTI.h"

class AssetManager {
private:
    std::unordered_map<std::string, Asset*> m_assetTree;

    // Le conteneur secret pour ne pas faire lagger le jeu !
    // Il associe l'UUID de l'asset à sa tâche de chargement en arrière-plan.
    std::unordered_map<std::string, std::future<void>> m_loadingTasks;

public:
    // ... tes autres méthodes (Register, etc.)

    // ---------------------------------------------------------
    // CHARGEMENT SYNCHRONE (Classique)
    // ---------------------------------------------------------
    void LoadSync(Asset* asset) {
        if (asset == nullptr || asset->objet != nullptr) return;

        asset->objet = Factory::Create(asset->type);

        if (asset->objet != nullptr) {
            JsonArchive archive(asset->path, ArchiveDirection::Read);
            asset->objet->Process(archive);
        }
    }

    // ---------------------------------------------------------
    // CHARGEMENT ASYNCHRONE (Ne bloque pas le jeu)
    // ---------------------------------------------------------
    void LoadAsync(Asset* asset) {
        if (asset == nullptr || asset->objet != nullptr) return;

        // 1. Sécurité : Vérifier qu'on n'est pas déjà en train de le charger
        // (Pour éviter de lancer 10 threads si le joueur clique 10 fois)
        if (m_loadingTasks.find(asset->uiid) != m_loadingTasks.end()) {
            return;
        }

        // 2. On lance le thread ET ON STOCKE LE RETOUR dans la map
        // std::launch::async force la création d'un vrai thread séparé
        m_loadingTasks[asset->uiid] = std::async(std::launch::async, [this, asset]() {
            this->LoadSync(asset);
            });
    }

    // ---------------------------------------------------------
    // MISE A JOUR (A appeler à chaque frame dans ton jeu)
    // ---------------------------------------------------------
    void Update() {
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
};