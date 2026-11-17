#include <unordered_map>
#include <thread>
#include <chrono>
#include <sstream>

#include "Guid.h"
#include "Checksum.h"
#include "JobSystem.h"
#include "AssetManager.h"
#include "Pawn.h"
#include "Log.h"
#include "Window.h"
#include "ImGUI.h"
#include "ImGUIManager.h"

int main() {
    // 1. Initialisation du système de log du moteur
    Log::Init("engine.log");

    LOG_INFO("========================================");
    LOG_INFO("   LANCEMENT DU MOTEUR DE JEU (SDL3)    ");
    LOG_INFO("========================================");

    // 2. Initialisation de la Fenêtre SDL3
    Window window(1280, 720, 1280, 720, "UI Asset Engine");
    if (!window.Init()) {
        LOG_ERROR("Erreur critique : Impossible d'initialiser la fenêtre SDL3 !");
        Log::Shutdown();
        return -1;
    }
    LOG_INFO("-> OK : Fenêtre SDL3 initialisée avec succès.");

    // 3. Initialisation de Dear ImGui via ton gestionnaire ImGUI
    ImGUIManager imguiManager;
    if (!imguiManager.Init(window.GetWindow(), window.GetRenderer())) {
        LOG_ERROR("Erreur critique : Impossible d'initialiser Dear ImGui !");
        Log::Shutdown();
        return -1;
    }
    LOG_INFO("-> OK : Dear ImGui initialisé et connecté au renderer.");

    // -------------------------------------------------------------
    // 4. Validation des briques logicielles (Guid, Checksum, JobSystem...)
    // -------------------------------------------------------------
    LOG_INFO("[Validation] Test des briques de base du moteur...");

    // Test Guid & Map
    Guid guid1 = Guid::Generate();
    Guid guid2 = Guid::Generate();
    std::unordered_map<Guid, std::string> mapTest;
    mapTest[guid1] = "Player_Pawn_Data";
    mapTest[guid2] = "Level_01_Data";
    if (mapTest.find(guid1) != mapTest.end()) {
        LOG_INFO("-> OK : std::unordered_map<Guid, ...> fonctionne avec le Checksum.");
    }

    // Test Checksum CRC32
    std::string fakeJsonContent = "{ \"type\": \"Pawn\", \"health\": 100 }";
    uint32_t checksumValue = Checksum::ComputeFromString(fakeJsonContent);
    LOG_INFO("-> OK : CRC32 calculé (Valeur : " + std::to_string(checksumValue) + ")");

    // Test JobSystem
    JobSystem mainJobSystem;
    mainJobSystem.Submit([]() {
        LOG_INFO("   -> [JobSystem Worker] Tâche en arrière-plan exécutée avec succès !");
        });

    // Test Objet de Jeu (Pawn & Arme polymorphe)
    Pawn myPawn;
    myPawn.health = 85;
    myPawn.speed = 12.5f;

    Weapon* myWeapon = new Weapon();
    myWeapon->damage = 75;
    myPawn.equippedWeapon = myWeapon;

    // Test AssetManager
    AssetManager assetManager;
    LOG_INFO("-> OK : AssetManager initialisé avec succès.");

    // -------------------------------------------------------------
    // 5. Boucle principale du moteur (Game Loop & Debug UI)
    // -------------------------------------------------------------
    LOG_INFO("Entrée dans la boucle principale du moteur...");
    bool isRunning = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (isRunning) {
        // Calcul du DeltaTime
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        lastTime = currentTime;
        float deltaTime = elapsed.count();

        // A. Gestion des événements (Clavier, Souris et transmission ImGui)
        isRunning = window.PollEvents(deltaTime);

        // B. Démarrage de la frame ImGui
        imguiManager.BeginFrame();

        // --- C. CRÉATION DE TON TABLEAU DE BORD DE DEBUG ---
        ImGui::Begin("Dashboard Asset Engine");

        ImGui::Text("Performances : %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Separator();

        // Inspecteur du Pawn en direct
        if (ImGui::CollapsingHeader("Inspecteur : Pawn Actif", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderInt("Santé (Health)", &myPawn.health, 0, 200);
            ImGui::SliderFloat("Vitesse (Speed)", &myPawn.speed, 1.0f, 50.0f);

            if (myPawn.equippedWeapon) {
                Weapon* weapon = SAFE_CAST(Weapon, myPawn.equippedWeapon);
                ImGui::DragInt("Dégâts Arme", &weapon->damage, 1, 0, 500);
            }

            if (ImGui::Button("Afficher l'état complet dans la console")) {
                myPawn.Print();
            }
        }

        // Contrôles JobSystem / Assets
        if (ImGui::CollapsingHeader("JobSystem & AssetManager")) {
            ImGui::Text("File d'attente des tâches : Active");
            if (ImGui::Button("Lancer un Job asynchrone")) {
                mainJobSystem.Submit([]() {
                    LOG_INFO("   -> [Job Dynamique] Exécuté à la demande depuis ImGui !");
                    });
            }
        }

        ImGui::End();
        // ---------------------------------------------------

        // D. Rendu Graphique
        // 1. Effacer l'écran en noir (ou couleur unie)
        SDL_SetRenderDrawColor(window.GetRenderer(), 30, 30, 35, 255);
        SDL_RenderClear(window.GetRenderer());

        // 2. Dessiner l'interface ImGui par-dessus
        imguiManager.Render(window.GetRenderer());

        // 3. Afficher le tout à l'écran
        SDL_RenderPresent(window.GetRenderer());
    }

    // -------------------------------------------------------------
    // 6. Fermeture et nettoyage propre du moteur
    // -------------------------------------------------------------
    LOG_INFO("Fermeture du moteur demandée...");

    imguiManager.Shutdown();
    Log::Shutdown();

    LOG_INFO("Moteur arrêté proprement. À bientôt !");
    return 0;
}