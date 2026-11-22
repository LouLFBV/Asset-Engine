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
#include <memory>
#include <vector>
#include <string>
#include <exception>

static std::string NormalizePath(const std::string& path)
{
    std::error_code error;

    std::filesystem::path normalized =
        std::filesystem::weakly_canonical(path, error);

    if (error)
    {
        normalized = std::filesystem::absolute(path, error);

        if (error)
            normalized = std::filesystem::path(path);
    }

    return normalized.lexically_normal().generic_string();
}

std::string ImGUIManager::ResolveAssetsDir(bool createIfMissing)
{
#ifdef PROJET_ASSET_ENGINE_ASSETS_DIR
    const std::filesystem::path targetDir(PROJET_ASSET_ENGINE_ASSETS_DIR);
#else
    const std::filesystem::path targetDir("ProjetAssetEngine/Assets");
#endif

    std::error_code ec;
    if (createIfMissing && !std::filesystem::exists(targetDir, ec))
    {
        std::filesystem::create_directories(targetDir, ec);
    }

    return NormalizePath(targetDir.string());
}

// Vérifie si un Pawn est déjà présent dans le monde.
static bool IsPawnAlreadyInWorld(
    Pawn* pawn,
    const std::string& assetPath,
    const std::vector<Pawn*>& worldPawns
)
{
    if (pawn == nullptr)
        return false;

    const std::string normalizedAssetPath =
        NormalizePath(assetPath);

    for (Pawn* worldPawn : worldPawns)
    {
        if (worldPawn == nullptr)
            continue;

        // Même instance en mémoire.
        if (worldPawn == pawn)
            return true;

        // Deux instances associées au même fichier.
        if (!worldPawn->sourceFilePath.empty() &&
            NormalizePath(worldPawn->sourceFilePath) ==
            normalizedAssetPath)
        {
            return true;
        }
    }

    return false;
}

// Retire un Pawn du monde sans détruire les objets appartenant
// à l'AssetManager.
static void RemovePawnFromWorld(
    size_t index,
    std::vector<Pawn*>& worldPawns,
    std::vector<std::unique_ptr<Pawn>>& ownedPawns,
    int& selectedPawnIndex
)
{
    LOG_INFO("RemovePawnFromWorld appelee !");
    if (index >= worldPawns.size())
        return;

    Pawn* pawnToRemove = worldPawns[index];

    // Retirer le Pawn du monde.
    worldPawns.erase(worldPawns.begin() + index);

    // Vérifier si une autre référence au Pawn existe encore.
    bool stillPresent =
        std::find(
            worldPawns.begin(),
            worldPawns.end(),
            pawnToRemove
        ) != worldPawns.end();

    // Détruire uniquement les Pawns possédés par ownedPawns,
    // et seulement s'ils ne sont plus présents dans le monde.
    if (!stillPresent)
    {
        auto it = std::find_if(
            ownedPawns.begin(),
            ownedPawns.end(),
            [pawnToRemove](const std::unique_ptr<Pawn>& pawn)
            {
                return pawn.get() == pawnToRemove;
            }
        );

        if (it != ownedPawns.end())
        {
            ownedPawns.erase(it);
        }
    }

    // Mise à jour de la sélection.
    if (worldPawns.empty())
    {
        selectedPawnIndex = -1;
    }
    else
    {
        if (selectedPawnIndex > static_cast<int>(index))
        {
            --selectedPawnIndex;
        }
        else if (selectedPawnIndex == static_cast<int>(index))
        {
            selectedPawnIndex = std::min(
                selectedPawnIndex,
                static_cast<int>(worldPawns.size()) - 1
            );
        }
    }

    LOG_INFO("Pawn retire du monde.");
}

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
        std::cerr
            << "Erreur : initialisation du backend SDL3 impossible !"
            << std::endl;

        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplSDLRenderer3_Init(renderer))
    {
        std::cerr
            << "Erreur : initialisation du backend SDLRenderer3 impossible !"
            << std::endl;

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

    // Création d'un nouveau Pawn.
    if (ImGui::Button("Creer un Pawn", ImVec2(-1, 0)))
    {
        auto newPawn = std::make_unique<Pawn>();

        // Valeurs par défaut.
        newPawn->health = 75;
        newPawn->speed = 10.0f;
        newPawn->equippedWeapon = new Weapon();

        // Ajouter le Pawn au monde.
        worldPawns.push_back(newPawn.get());

        // Transférer sa propriété à ownedPawns.
        ownedPawns.push_back(std::move(newPawn));

        // Sélectionner automatiquement le nouveau Pawn.
        m_selectedPawnIndex =
            static_cast<int>(worldPawns.size()) - 1;

        LOG_INFO("Nouveau Pawn cree et ajoute au monde.");
    }

    ImGui::Separator();

    ImGui::Text(
        "Pawns actifs dans le monde (%zu) :",
        worldPawns.size()
    );

    // Liste des Pawns.
    int pawnIndexToRemove = -1;

    if (ImGui::BeginListBox("##PawnList", ImVec2(-1, 120)))
    {
        for (size_t i = 0; i < worldPawns.size(); ++i)
        {
            Pawn* pawn = worldPawns[i];

            if (!pawn)
                continue;

            ImGui::PushID(static_cast<int>(i));

            std::stringstream ss;

            if (!pawn->sourceFilePath.empty())
            {
                std::filesystem::path p(pawn->sourceFilePath);

                ss << p.stem().string()
                    << " (Sante : " << pawn->health << ")";
            }
            else
            {
                ss << "Pawn_Custom #" << i
                    << " (Sante : " << pawn->health << ")";
            }

            bool selected = m_selectedPawnIndex == static_cast<int>(i);

            if (ImGui::Selectable(ss.str().c_str(), selected))
            {
                m_selectedPawnIndex = static_cast<int>(i);
            }
            ImGui::PopID();

            if (pawnIndexToRemove != -1)
                break;
        }

        ImGui::EndListBox();
    }

    // Suppression après la liste.
    if (pawnIndexToRemove != -1)
    {
        RemovePawnFromWorld(
            static_cast<size_t>(pawnIndexToRemove),
            worldPawns,
            ownedPawns,
            m_selectedPawnIndex
        );
    }

    if (worldPawns.empty())
    {
        m_selectedPawnIndex = -1;
        return;
    }

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

    if (ImGui::Button("Sauvegarder en JSON"))
    {
        std::filesystem::path path;

        if (!currentPawn->sourceFilePath.empty())
        {
            path = currentPawn->sourceFilePath;
        }
        else
        {
            const std::string assetsDir = ResolveAssetsDir(true);
            path = std::filesystem::path(assetsDir) /
                ("pawn_" + std::to_string(m_selectedPawnIndex));
        }

        path.replace_extension(".json");

        try
        {
            JsonArchive archive(
                path.string(),
                ArchiveDirection::Write
            );

            currentPawn->Process(archive);

            if (currentPawn->sourceFilePath.empty())
            {
                currentPawn->sourceFilePath = path.string();
            }

            LOG_INFO(
                ("Sauvegarde JSON : " + path.string()).c_str()
            );
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(
                ("Erreur sauvegarde JSON : " +
                    std::string(e.what())).c_str()
            );
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Sauvegarder en Binaire"))
    {
        std::filesystem::path path;

        if (!currentPawn->sourceFilePath.empty())
        {
            path = currentPawn->sourceFilePath;
        }
        else
        {
            const std::string assetsDir = ResolveAssetsDir(true);
            path = std::filesystem::path(assetsDir) /
                ("pawn_" + std::to_string(m_selectedPawnIndex));
        }

        path.replace_extension(".bin");

        try
        {
            BinaryArchive archive(
                path.string(),
                ArchiveDirection::Write
            );

            currentPawn->Process(archive);

            if (currentPawn->sourceFilePath.empty())
            {
                currentPawn->sourceFilePath = path.string();
            }

            LOG_INFO(
                ("Sauvegarde binaire : " + path.string()).c_str()
            );
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(
                ("Erreur sauvegarde binaire : " +
                    std::string(e.what())).c_str()
            );
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Retirer du monde", ImVec2(-1, 0)))
    {
        RemovePawnFromWorld(
            static_cast<size_t>(m_selectedPawnIndex),
            worldPawns,
            ownedPawns,
            m_selectedPawnIndex
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
        assetManager.ScanDirectory(ResolveAssetsDir(false), worldPawns);

        if (worldPawns.empty())
        {
            m_selectedPawnIndex = -1;
        }
        else
        {
            m_selectedPawnIndex = std::clamp(
                m_selectedPawnIndex,
                0,
                static_cast<int>(worldPawns.size()) - 1
            );
        }

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

        ImGui::BulletText(
            "Path : %s",
            asset->path.c_str()
        );

        ImGui::SameLine();

        RTTI* loadedObject = asset->GetObject();

        Pawn* assetPawn = SAFE_CAST(Pawn, loadedObject);

        bool alreadyInWorld = false;

        for (Pawn* worldPawn : worldPawns)
        {
            if (!worldPawn)
                continue;

            // Même instance.
            if (assetPawn != nullptr && worldPawn == assetPawn)
            {
                alreadyInWorld = true;
                break;
            }

            // Même fichier source.
            if (!worldPawn->sourceFilePath.empty() &&
                NormalizePath(worldPawn->sourceFilePath) ==
                NormalizePath(asset->path))
            {
                alreadyInWorld = true;
                break;
            }
        }

        if (alreadyInWorld)
        {
            ImGui::TextDisabled("[Deja dans le monde]");
        }
        else if (assetPawn != nullptr)
        {
            ImGui::TextColored(
                ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                "[Charge]"
            );

            ImGui::SameLine();

            if (ImGui::Button("Ajouter au monde"))
            {
                // Vérification supplémentaire avant l'ajout.
                if (!IsPawnAlreadyInWorld(
                    assetPawn,
                    asset->path,
                    worldPawns))
                {
                    assetPawn->sourceFilePath = asset->path;

                    worldPawns.push_back(assetPawn);

                    LOG_INFO("Pawn ajoute au monde.");
                }
                else
                {
                    LOG_INFO("Ce Pawn est deja present dans le monde.");
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

    ImGui::Render();

    ImGui_ImplSDLRenderer3_RenderDrawData(
        ImGui::GetDrawData(),
        renderer
    );
}