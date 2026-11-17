#include "ImGUIManager.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "AssetManager.h"
#include "Asset.h"
#include "RTTI.h"
#include "Log.h"
#include "JsonArchive.h"
#include "BinaryArchive.h"

#include <algorithm>
#include <sstream>
#include <filesystem>

#include <iostream>

bool ImGUIManager::Init(SDL_Window* window, SDL_Renderer* renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.5f;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5f);

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
    {
        std::cerr << "Erreur : initialisation du backend SDL3 impossible !" << std::endl;
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplSDLRenderer3_Init(renderer))
    {
        std::cerr << "Erreur : initialisation du backend SDLRenderer3 impossible !" << std::endl;

        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    m_initialized = true;

    return true;
}

void ImGUIManager::Shutdown()
{
    if (!m_initialized)
        return;

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();

    m_initialized = false;
}

void ImGUIManager::BeginFrame()
{
    if (!m_initialized)
        return;

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();
}

void ImGUIManager::ApplyStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 5.0f;
	style.FrameRounding = 3.0f;
}



void ImGUIManager::DrawDashboard(
	AssetManager& assetManager,
	std::vector<Pawn*>& worldPawns,
	std::vector<std::unique_ptr<Pawn>>& ownedPawns
)
{
	ApplyStyle();

	ImGui::SetNextWindowSize(
		ImVec2(750, 800),
		ImGuiCond_FirstUseEver
	);

	ImGui::Begin("Dashboard - Moteur de Jeu C++");

	ImGui::TextColored(
		ImVec4(0.0f, 0.8f, 1.0f, 1.0f),
		"Moteur actif & connecte"
	);

	ImGui::Text(
		"FPS : %.1f",
		ImGui::GetIO().Framerate
	);

	ImGui::Separator();

	DrawPawnPanel(worldPawns, ownedPawns);

	DrawAssetPanel(assetManager, worldPawns);

	ImGui::End();
}

void ImGUIManager::DrawPawnPanel(
    std::vector<Pawn*>& worldPawns,
    std::vector<std::unique_ptr<Pawn>>& ownedPawns
)
{
    if (!ImGui::CollapsingHeader(
        "Gestion des Pawns",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    ImGui::Text(
        "Pawns actifs dans le monde (%zu) :",
        worldPawns.size()
    );

    // Liste des Pawns.
    if (ImGui::BeginListBox("##PawnList", ImVec2(-1, 90)))
    {
        for (size_t i = 0; i < worldPawns.size(); ++i)
        {
            Pawn* pawn = worldPawns[i];

            if (!pawn)
                continue;

            std::stringstream ss;

            ss << "Pawn #" << i
                << " (Sante : " << pawn->health << ")";

            bool selected =
                m_selectedPawnIndex == static_cast<int>(i);

            if (ImGui::Selectable(ss.str().c_str(), selected))
            {
                m_selectedPawnIndex = static_cast<int>(i);
            }
        }

        ImGui::EndListBox();
    }

    // Creation d'un nouveau Pawn.
    if (ImGui::Button("Creer un nouveau Pawn", ImVec2(-1, 0)))
    {
        auto newPawn = std::make_unique<Pawn>();

        newPawn->health = 75;
        newPawn->speed = 10.0f;

        worldPawns.push_back(newPawn.get());

        ownedPawns.push_back(std::move(newPawn));

        m_selectedPawnIndex =
            static_cast<int>(worldPawns.size() - 1);

        LOG_INFO("Nouveau Pawn ajoute au monde.");
    }

    // Inspection du Pawn selectionne.
    if (worldPawns.empty())
        return;

    if (m_selectedPawnIndex < 0 ||
        m_selectedPawnIndex >= static_cast<int>(worldPawns.size()))
    {
        m_selectedPawnIndex = 0;
    }

    Pawn* currentPawn = worldPawns[m_selectedPawnIndex];

    if (!currentPawn)
        return;

    ImGui::Separator();

    ImGui::Text(
        "Inspecteur du Pawn [#%d]",
        m_selectedPawnIndex
    );

    if (!currentPawn->sourceFilePath.empty())
    {
        ImGui::TextColored(
            ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "Source : %s",
            currentPawn->sourceFilePath.c_str()
        );
    }

    ImGui::SliderInt(
        "Sante (Health)",
        &currentPawn->health,
        0,
        200
    );

    ImGui::SliderFloat(
        "Vitesse (Speed)",
        &currentPawn->speed,
        1.0f,
        50.0f
    );

    if (currentPawn->equippedWeapon)
    {
        Weapon* weapon =
            SAFE_CAST(Weapon, currentPawn->equippedWeapon);

        if (weapon)
        {
            ImGui::DragInt(
                "Degats Arme",
                &weapon->damage,
                1,
                0,
                500
            );
        }
    }

    // Sauvegarde JSON.
    if (ImGui::Button("Sauvegarder en JSON"))
    {
        std::string filename =
            !currentPawn->sourceFilePath.empty()
            ? currentPawn->sourceFilePath
            : "Assets/pawn_" +
            std::to_string(m_selectedPawnIndex) + ".json";

        std::filesystem::path path(filename);
        path.replace_extension(".json");

        filename = path.string();

        JsonArchive archive(
            filename,
            ArchiveDirection::Write
        );

        currentPawn->Process(archive);

        LOG_INFO(
            ("Sauvegarde JSON : " + filename).c_str()
        );
    }

    ImGui::SameLine();

    // Sauvegarde binaire.
    if (ImGui::Button("Sauvegarder en Binaire"))
    {
        std::string filename =
            !currentPawn->sourceFilePath.empty()
            ? currentPawn->sourceFilePath
            : "Assets/pawn_" +
            std::to_string(m_selectedPawnIndex) + ".bin";

        std::filesystem::path path(filename);
        path.replace_extension(".bin");

        filename = path.string();

        BinaryArchive archive(
            filename,
            ArchiveDirection::Write
        );

        currentPawn->Process(archive);

        LOG_INFO(
            ("Sauvegarde binaire : " + filename).c_str()
        );
    }
}

void ImGUIManager::DrawAssetPanel(
    AssetManager& assetManager,
    std::vector<Pawn*>& worldPawns
)
{
    if (!ImGui::CollapsingHeader(
        "Asset Registry & JobSystem (Async)",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (ImGui::Button("Re-scanner le dossier Assets"))
    {
        assetManager.ScanDirectory("Assets");

        // Attention : les Pawns provenant de l'ancien
        // registre ne doivent plus etre references ici.
        // Il faut les retirer avant de les detruire.
        LOG_INFO("Re-scan du dossier Assets demande.");
    }

    ImGui::Separator();

    ImGui::Text("Fichiers indexes (sans chargement lourd) :");

    for (auto& pair : assetManager.GetRegistry())
    {
        Asset* asset = pair.second;

        if (!asset || asset->path.empty())
            continue;

        ImGui::PushID(asset);

        ImGui::BulletText("Path : %s", asset->path.c_str());

        ImGui::SameLine();

        if (asset->IsLoaded())
        {
            ImGui::TextColored(
                ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                "[Charge]"
            );

            ImGui::SameLine();

            if (ImGui::Button("Ajouter au monde"))
            {
                Pawn* loadedPawn =
                    SAFE_CAST(Pawn, asset->GetObject());

                if (loadedPawn)
                {
                    bool alreadyExists =
                        std::find(
                            worldPawns.begin(),
                            worldPawns.end(),
                            loadedPawn
                        ) != worldPawns.end();

                    if (!alreadyExists)
                    {
                        loadedPawn->sourceFilePath = asset->path;

                        worldPawns.push_back(loadedPawn);

                        LOG_INFO("Pawn ajoute au monde.");
                    }
                    else
                    {
                        LOG_INFO("Ce Pawn est deja present.");
                    }
                }
            }
        }
        else
        {
            if (ImGui::Button("Load Async"))
            {
                assetManager.LoadAsync(asset);
            }

            ImGui::SameLine();

            ImGui::TextDisabled("[Non charge]");
        }

        ImGui::PopID();
    }
}

void ImGUIManager::Render(
    SDL_Renderer* renderer,
    AssetManager& assetManager,
    std::vector<Pawn*>& worldPawns,
    std::vector<std::unique_ptr<Pawn>>& ownedPawns
)
{
    if (!m_initialized || renderer == nullptr)
        return;

    DrawDashboard(
        assetManager,
        worldPawns,
        ownedPawns
    );

    // Termine la frame ImGui.
    ImGui::Render();

    // Envoie les donnees au renderer SDL3.
    ImGui_ImplSDLRenderer3_RenderDrawData(
        ImGui::GetDrawData(),
        renderer
    );
}