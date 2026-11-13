#include <iostream>
#include <thread>
#include "Log.h"
#include "Checksum.h"
#include "JsonArchive.h"
#include "JobSystem.h"
#include "AssetManager.h"
#include "Pawn.h"

int main() {
    //assert(0 == 1);
    // 1. Initialisation du Log
    Log::Init("engine_job_test.log");
    LOG_INFO("=== DEBUT DU TEST AVEC JOBSYSTEM ===");

    // 2. Démarrage du JobSystem (ex: avec tous les cœurs disponibles du CPU)
    int numThreads = std::max(2u, std::thread::hardware_concurrency());
    LOG_INFO("Demarrage du JobSystem avec " + std::to_string(numThreads) + " threads workers.");

    // 3. Initialisation de l'AssetManager en lui injectant le JobSystem
    // (Adapte cette ligne selon la façon dont tu passes ton JobSystem à l'AssetManager)
    AssetManager assetManager{};

    const std::string filepath = "pawn_job_test.json";

    // 4. Création d'un fichier de sauvegarde valide sur le disque
    {
        LOG_INFO("Creation du fichier de test valide...");
        Pawn hero;
        hero.health = 100;
        hero.speed = 15.0f;
        hero.inventory = { 7, 77, 777 };
        hero.stats["Agility"] = 50;

        Weapon* bow = new Weapon();
        bow->damage = 45;
        hero.equippedWeapon = bow;

        JsonArchive writer(filepath, ArchiveDirection::Write);
        hero.Process(writer);
    }

    // 5. Configuration de l'Asset
    Asset heroAsset;
    heroAsset.uiid = heroAsset.uiid.Generate();
    heroAsset.path = filepath;
    heroAsset.type = "Pawn";
    heroAsset.objet = nullptr;
    heroAsset.checksum = Checksum::ComputeFromFile(filepath);

    // 6. Test du chargement Asynchrone via le JobSystem
    LOG_INFO("Lancement du chargement ASYNCHRONE via le JobSystem...");
    assetManager.LoadAsync(&heroAsset);

    // Simulation de la boucle de jeu ou attente active que le JobSystem termine
    LOG_INFO("Le jeu continue de tourner pendant que les threads travaillent...");

    // Tu peux soit utiliser jobSystem.WaitIdle() pour attendre que tous les jobs soient finis :
    assetManager.GetJobSystem().WaitIdle();

    // 7. Vérification du résultat
    if (heroAsset.objet != nullptr) {
        LOG_INFO("Chargement termine avec succes par le pool de threads ! Affichage :");
        Pawn* loadedHero = dynamic_cast<Pawn*>(heroAsset.objet);
        if (loadedHero) {
            loadedHero->Print();
        }
    }
    else {
        LOG_ERROR("Erreur : L'asset n'a pas pu être chargé.");
    }

    // 8. Test de sécurité du Checksum sur un fichier corrompu
    LOG_INFO("--- TEST DE SECURITE : Fichier corrompu ---");
    {
        std::ofstream corrupt(filepath, std::ios::app);
        corrupt << "\n// Corruption volontaire";
    }

    Asset corruptAsset;
    corruptAsset.uiid = corruptAsset.uiid.Generate();
    corruptAsset.path = filepath;
    corruptAsset.type = "Pawn";
    corruptAsset.objet = nullptr;
    corruptAsset.checksum = heroAsset.checksum; // Ancien checksum valide

    // Tentative de chargement synchrone (doit bloquer et logger une erreur)
    assetManager.LoadSync(&corruptAsset);

    // 9. Fermeture propre du moteur
    LOG_INFO("=== FIN DU TEST JOBSYSTEM AVEC SUCCES ===");
    Log::Shutdown();
    //{
    //    Guid key{};
    //    // es ce que tu est hashable ? non throw oui je prend le hash
    //    using ghash = std::hash<Guid>;
    //    std::size_t id = ghash::operator()(key);




    //}
    
    // Le destructeur de jobSystem s'exécutera ici, fermant proprement les threads workers.
    return 0;
}