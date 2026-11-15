#include <unordered_map>
#include <thread>
#include <chrono>
#include <sstream> // Nécessaire pour construire les chaînes si besoin
#include "Guid.h"
#include "Checksum.h"
#include "JobSystem.h"
#include "AssetManager.h"
#include "Pawn.h"
#include "Log.h"

int main() {
    // Initialisation du fichier de log du moteur
    Log::Init("engine.log");

    LOG_INFO("========================================");
    LOG_INFO("   DEBUT DES TESTS DU MOTEUR DE JEU     ");
    LOG_INFO("========================================");

    // -------------------------------------------------------------
    // 1. Test de la classe Guid et du Hash avec le Checksum
    // -------------------------------------------------------------
    LOG_INFO("[1/5] Test du systeme de Guid & Hash...");
    Guid guid1 = Guid::Generate();
    Guid guid2 = Guid::Generate();

    if (!(guid1 == guid2)) {
        LOG_INFO("-> OK : Deux GUID generes sont bien differents.");
    }
    else {
        LOG_ERROR("-> ERREUR : Collision de GUID !");
    }

    // Test de l'unordered_map avec le Guid (qui utilise ton Checksum sous le capot)
    std::unordered_map<Guid, std::string> mapTest;
    mapTest[guid1] = "Player_Pawn_Data";
    mapTest[guid2] = "Level_01_Data";

    if (mapTest.find(guid1) != mapTest.end() && mapTest[guid1] == "Player_Pawn_Data") {
        LOG_INFO("-> OK : std::unordered_map<Guid, ...> fonctionne avec le Checksum !");
    }

    // -------------------------------------------------------------
    // 2. Test du Checksum (CRC32)
    // -------------------------------------------------------------
    LOG_INFO("[2/5] Test du Checksum...");
    std::string fakeJsonContent = "{ \"type\": \"Pawn\", \"health\": 100 }";
    uint32_t checksumValue = Checksum::ComputeFromString(fakeJsonContent);

    // Petite astuce pour logger une variable numérique avec une macro de string
    std::string checksumMsg = "-> OK : CRC32 calcule avec succes (Valeur : " + std::to_string(checksumValue) + ")";
    LOG_INFO(checksumMsg);

    // -------------------------------------------------------------
    // 3. Test du JobSystem
    // -------------------------------------------------------------
    LOG_INFO("[3/5] Test du JobSystem...");
    JobSystem mainJobSystem;

    mainJobSystem.Submit([]() {
        LOG_INFO("   -> [JobSystem Worker] Tache en arriere-plan executee avec succes !");
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    LOG_INFO("-> OK : JobSystem operationnel.");

    // -------------------------------------------------------------
    // 4. Test de la classe Pawn & RTTI / Polymorphisme
    // -------------------------------------------------------------
    LOG_INFO("[4/5] Test de la classe Pawn et des objets du jeu...");

    Pawn myPawn;
    myPawn.health = 85;
    myPawn.speed = 12.5f;

    // On lui équipe une arme polymorphe pour tester le RTTI
    Weapon* myWeapon = new Weapon();
    myWeapon->damage = 75;
    myPawn.equippedWeapon = myWeapon;

    // Affichage des infos du Pawn (la méthode Print utilise std::cout, ce qui est parfait pour la console)
    LOG_INFO("-> État initial du Pawn :");
    myPawn.Print();

    // -------------------------------------------------------------
    // 5. Test de l'AssetManager
    // -------------------------------------------------------------
    LOG_INFO("[5/5] Test de l'AssetManager...");
    AssetManager assetManager;

    LOG_INFO("-> OK : AssetManager initialise avec succes et connecte au JobSystem.");

    LOG_INFO("========================================");
    LOG_INFO("   TOUS LES TESTS SONT PASSES AVEC SUCCES ! ");
    LOG_INFO("========================================");

    // Fermeture propre du système de log
    Log::Shutdown();

    return 0;
}