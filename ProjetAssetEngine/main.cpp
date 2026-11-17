#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

#include "Guid.h"
#include "Checksum.h"
#include "JobSystem.h"
#include "AssetManager.h"
#include "Asset.h"
#include "Pawn.h"
#include "Log.h"
#include "Window.h"
#include "ImGUIManager.h"
#include "imgui.h"
#include "JsonArchive.h"
#include "BinaryArchive.h"

int main() {
    // 1. Initialisation du système de log du moteur
    Log::Init("engine.log");

    LOG_INFO("========================================");
    LOG_INFO("   LANCEMENT DU MOTEUR DE JEU (SDL3)    ");
    LOG_INFO("========================================");

    // 2. Initialisation de la Fenêtre SDL3 (1600x900)
    Window window(1600, 900, 1600, 900, "Gaming Campus - Moteur C++");
    if (!window.Init()) {
        LOG_ERROR("Erreur critique : Impossible d'initialiser la fenêtre SDL3 !");
        Log::Shutdown();
        return -1;
    }
    LOG_INFO("-> OK : Fenêtre SDL3 initialisée avec succès.");

    // 3. Initialisation de Dear ImGui
    ImGUIManager imguiManager;
    if (!imguiManager.Init(window.GetWindow(), window.GetRenderer())) {
        LOG_ERROR("Erreur critique : Impossible d'initialiser Dear ImGui !");
        Log::Shutdown();
        return -1;
    }
    LOG_INFO("-> OK : Dear ImGui initialisé et connecté au renderer.");

    // 4. Initialisation de l'AssetManager et Scan automatique du dossier "Assets"
    AssetManager assetManager;
    assetManager.ScanDirectory("Assets");

    // Collection de Pawns actifs dans le monde
    std::vector<Pawn*> worldPawns;
    int selectedPawnIndex = 0;

    // Création d'un Pawn de test initial
    Pawn* playerPawn = new Pawn();
    playerPawn->health = 100;
    playerPawn->speed = 15.0f;
    playerPawn->equippedWeapon = new Weapon();
    Weapon* weaponPlayer = SAFE_CAST(Weapon, playerPawn->equippedWeapon);
    if (weaponPlayer) weaponPlayer->damage = 50;
    worldPawns.push_back(playerPawn);

    // -------------------------------------------------------------
    // 5. Boucle principale du moteur (Game Loop & Debug UI)
    // -------------------------------------------------------------
    LOG_INFO("Entrée dans la boucle principale du moteur...");
    bool isRunning = true;
    //auto lastTime = std::chrono::high_resolution_clock::now();

    while (isRunning) {
        //auto currentTime = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<float> elapsed = currentTime - lastTime;
        //lastTime = currentTime;

        // A. Gestion des événements (DOIT impérativement intercepter la fermeture SDL)
        isRunning = window.PollEvents();

        // --- PETITE SÉCURITÉ ANTI-SURCHAUFFE (Optionnel mais recommandé) ---
        // Si PollEvents retourne false, on sort de la boucle immédiatement !
        if (!isRunning) break;

        // B. Démarrage de la frame ImGui
        imguiManager.BeginFrame();

        // --- C. DASHBOARD IMGUI ---
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.0f;
        style.FrameRounding = 3.0f;

        // Taille par défaut et redimensionnement libre de la fenêtre ImGui
        ImGui::SetNextWindowSize(ImVec2(750, 800), ImGuiCond_FirstUseEver);

        ImGui::Begin("Dashboard - Moteur de Jeu C++", nullptr);

        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Moteur actif & connecte");
        ImGui::Text("FPS : %.1f", ImGui::GetIO().Framerate);
        ImGui::Separator();

        // --- ONGLET 1 : GESTION DES PAWNS ---
        if (ImGui::CollapsingHeader("Gestion des Pawns", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Pawns actifs dans le monde (%zu) :", worldPawns.size());

            if (ImGui::BeginListBox("##PawnList", ImVec2(-1, 90))) {
                for (size_t i = 0; i < worldPawns.size(); ++i) {
                    std::stringstream ss;
                    ss << "Pawn #" << i << " (Sante: " << worldPawns[i]->health << ")";
                    bool isSelected = (selectedPawnIndex == (int)i);
                    if (ImGui::Selectable(ss.str().c_str(), isSelected)) {
                        selectedPawnIndex = (int)i;
                    }
                }
                ImGui::EndListBox();
            }

            if (ImGui::Button("Creer un nouveau Pawn", ImVec2(-1, 0))) {
                Pawn* newPawn = new Pawn();
                newPawn->health = 75;
                newPawn->speed = 10.0f;
                worldPawns.push_back(newPawn);
                selectedPawnIndex = (int)(worldPawns.size() - 1);
                LOG_INFO("-> Nouveau Pawn ajoute à la liste.");
            }

            // Inspecteur du Pawn sélectionné
            if (!worldPawns.empty() && selectedPawnIndex >= 0 && selectedPawnIndex < (int)worldPawns.size()) {
                Pawn* currentPawn = worldPawns[selectedPawnIndex];

                ImGui::Separator();
                ImGui::Text("Inspecteur du Pawn [#%d]", selectedPawnIndex);
                ImGui::SliderInt("Sante (Health)", &currentPawn->health, 0, 200);
                ImGui::SliderFloat("Vitesse (Speed)", &currentPawn->speed, 1.0f, 50.0f);

                if (currentPawn->equippedWeapon) {
                    Weapon* weapon = SAFE_CAST(Weapon, currentPawn->equippedWeapon);
                    if (weapon) {
                        ImGui::DragInt("Degats Arme", &weapon->damage, 1, 0, 500);
                    }
                }

                if (ImGui::Button("Sauvegarder en JSON")) {
                    std::string filename = "Assets/pawn_" + std::to_string(selectedPawnIndex) + ".json";
                    JsonArchive jsonAr(filename, ArchiveDirection::Write);
                    currentPawn->Process(jsonAr);
                    LOG_INFO(("-> Pawn sauvegarde : " + filename).c_str());
                }
                ImGui::SameLine();
                if (ImGui::Button("Sauvegarder en Binaire")) {
                    std::string filename = "Assets/pawn_" + std::to_string(selectedPawnIndex) + ".bin";
                    BinaryArchive binAr(filename, ArchiveDirection::Write);
                    currentPawn->Process(binAr);
                    LOG_INFO(("-> Pawn sauvegarde binaire : " + filename).c_str());
                }
            }
        }

        // --- ONGLET 2 : ASSET REGISTRY & JOBYSTEM (ASYNC) ---
        if (ImGui::CollapsingHeader("Asset Registry & JobSystem (Async)", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Re-scanner le dossier Assets")) {
                assetManager.ScanDirectory("Assets");
            }

            ImGui::Separator();
            ImGui::Text("Fichiers indexes (Sans chargement lourd) :");

            for (auto& pair : assetManager.GetRegistry()) {
                Asset* asset = pair.second;

                if (asset == nullptr) continue;
                if (asset->path.empty()) continue;

                ImGui::BulletText("Path: %s", asset->path.c_str());
                ImGui::SameLine();

                // Si l'asset est chargé en mémoire par le JobSystem
                if (asset->objet != nullptr) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Charge]");
                    ImGui::SameLine();

                    // BOUTON POUR AJOUTER LE PAWN CHARGÉ DANS LE MONDE DU JEU !
                    std::string spawnLabel = "Ajouter au monde##" + asset->path;
                    if (ImGui::Button(spawnLabel.c_str())) {
                        Pawn* loadedPawn = SAFE_CAST(Pawn, asset->objet);
                        if (loadedPawn) {
                            worldPawns.push_back(loadedPawn);
                            LOG_INFO("-> Pawn charge depuis le registre ajoute aux entites du monde !");
                        }
                    }
                }
                else {
                    // Bouton pour lancer le chargement asynchrone
                    std::string btnLabel = "Load Async##" + asset->path;
                    if (ImGui::Button(btnLabel.c_str())) {
                        assetManager.LoadAsync(asset);
                    }
                }
            }
        }

        ImGui::End();
        // ---------------------------------------------------

        // D. Rendu Graphique SDL3 & ImGui
        SDL_SetRenderDrawColor(window.GetRenderer(), 30, 30, 35, 255);
        SDL_RenderClear(window.GetRenderer());

        imguiManager.Render(window.GetRenderer());

        SDL_RenderPresent(window.GetRenderer());
    }

    // -------------------------------------------------------------
    // 6. Nettoyage et fermeture propre
    // -------------------------------------------------------------
    LOG_INFO("Fermeture du moteur demandée...");

    // Vider la liste du monde SANS faire de delete direct sur les pawns chargés par l'AssetManager 
    // (pour éviter la double suppression, vu qu'ils appartiennent au registre)
    worldPawns.clear();

    // Nettoyage sécurisé de l'Asset Registry (qui supprime les objets et les assets proprement)
    for (auto& pair : assetManager.GetRegistry()) {
        if (pair.second != nullptr) {
            if (pair.second->objet != nullptr) {
                delete pair.second->objet; // Supprime le Pawn sous-jacent
                pair.second->objet = nullptr;
            }
            delete pair.second; // Supprime l'Asset du registre
            pair.second = nullptr;
        }
    }
    assetManager.GetRegistry().clear();

    imguiManager.Shutdown();
    Log::Shutdown();

    LOG_INFO("Moteur arrêté proprement. À bientôt !");
    return 0;
}