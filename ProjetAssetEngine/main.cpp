#include <iostream>
#include "AssetManager.h"
#include "JsonArchive.h"
#include "Pawn.h"

int main() {
    AssetManager manager;
    Asset myAsset = { "hero_01", "pawn_data.json", "Pawn", nullptr };

    std::cout << "Lancement du chargement ASYNCHRONE...\n";
    manager.LoadAsync(&myAsset);

    // Simulation de la boucle de jeu (Tourne à 60 FPS)
    bool isGameRunning = true;
    int frameCount = 0;

    while (isGameRunning) {
        // 1. Mise à jour du gestionnaire de ressources
        manager.Update();

        // 2. Logique du jeu
        if (myAsset.objet != nullptr) {
            std::cout << "\n[FRAME " << frameCount << "] L'asset est enfin DISPONIBLE ! On peut l'utiliser.\n";

            Pawn* p = dynamic_cast<Pawn*>(myAsset.objet);
            p->Print();

            isGameRunning = false; // On quitte la boucle pour l'exemple
        }
        else {
            // Le jeu continue de tourner et de s'afficher sans lagger !
            std::cout << "[FRAME " << frameCount << "] Le jeu tourne... (chargement en cours)\n";
        }

        frameCount++;
    }

    return 0;
}