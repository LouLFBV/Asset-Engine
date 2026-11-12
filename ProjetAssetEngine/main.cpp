#include <iostream>
#include <fstream>
#include "Log.h"
#include "Checksum.h"
#include "JsonArchive.h"
#include "AssetManager.h"
#include "Pawn.h"

int main() {
    // 1. Initialiser le système de Log centralisé
    Log::Init("engine_test.log");
    LOG_INFO("=== DEBUT DU TEST GLOBAL DU MOTEUR ===");

    const std::string filepath = "pawn_test_data.json";

    // 2. Création d'un fichier de données valide sur le disque
    {
        LOG_INFO("Creation du fichier de sauvegarde valide...");
        Pawn originalPawn;
        originalPawn.health = 90;
        originalPawn.speed = 12.5f;
        originalPawn.inventory = { 101, 202, 303 };
        originalPawn.stats["Strength"] = 40;

        // Arme polymorphe
        Weapon* sword = new Weapon();
        sword->damage = 60;
        originalPawn.equippedWeapon = sword;

        JsonArchive writer(filepath, ArchiveDirection::Write);
        originalPawn.Process(writer);
    }

    // 3. Préparer l'Asset Manager et l'Asset
    AssetManager manager;
    Asset myHeroAsset;
    myHeroAsset.uiid = "hero_player_1";
    myHeroAsset.path = filepath;
    myHeroAsset.type = "Pawn";
    myHeroAsset.objet = nullptr;

    // Calcul et stockage du checksum initial (sécurité)
    myHeroAsset.checksum = Checksum::ComputeFromFile(filepath);
    LOG_INFO("Checksum calcule et enregistre pour l'asset : " + std::to_string(myHeroAsset.checksum));

    // 4. Test du chargement Asynchrone (avec la boucle de jeu / Update)
    LOG_INFO("Lancement du chargement ASYNCHRONE...");
    manager.LoadAsync(&myHeroAsset);

    bool isRunning = true;
    int frameCount = 0;

    // Simulation d'une boucle de jeu à 60 FPS
    while (isRunning && frameCount < 100) {
        // Mise à jour des tâches en arrière-plan
        manager.Update();

        if (myHeroAsset.objet != nullptr) {
            LOG_INFO("Asset charge avec succes en asynchrone ! Affichage :");

            // Cast sécurisé pour afficher les données du Pawn
            Pawn* loadedPawn = dynamic_cast<Pawn*>(myHeroAsset.objet);
            if (loadedPawn) {
                loadedPawn->Print();
            }

            isRunning = false; // On arrête la boucle de test de chargement
        }
        else {
            // Le jeu continue de tourner sans freeze
            frameCount++;
        }
    }

    // 5. Test ultime : La sécurité du Checksum face à une corruption
    LOG_INFO("--- TEST DE SECURITE : Corruption du fichier ---");
    {
        // On écrit de force dans le fichier pour modifier son contenu et fausser son empreinte
        std::ofstream corruptFile(filepath, std::ios::app);
        corruptFile << "\n// Fichier corrompu par un tiers";
    }

    // On crée un deuxième asset pointant vers ce fichier corrompu
    Asset corruptedAsset;
    corruptedAsset.uiid = "hero_corrupt";
    corruptedAsset.path = filepath;
    corruptedAsset.type = "Pawn";
    corruptedAsset.objet = nullptr;
    corruptedAsset.checksum = myHeroAsset.checksum; // On garde l'ancien checksum valide

    LOG_INFO("Tentative de chargement du fichier corrompu (doit declencher une erreur de Checksum)...");
    manager.LoadSync(&corruptedAsset); // Doit échouer et afficher LOG_ERROR dans la console

    // 6. Fermeture propre du moteur
    LOG_INFO("=== FIN DU TEST GLOBAL AVEC SUCCES ===");
    Log::Shutdown();

    return 0;
}