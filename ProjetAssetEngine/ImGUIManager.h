
#pragma once

#include <SDL3/SDL.h>

#include <vector>
#include <memory>

#include "Pawn.h"

class AssetManager;

class ImGUIManager
{
public:

    ImGUIManager() = default;
    ~ImGUIManager() = default;

    bool Init(SDL_Window* window, SDL_Renderer* renderer);

    void Shutdown();

    void BeginFrame() const;

    void Render(SDL_Renderer* renderer, AssetManager& assetManager, std::vector<Pawn*>& worldPawns, std::vector<std::unique_ptr<Pawn>>& ownedPawns);
    static std::string ResolveAssetsDir(bool createIfMissing = false);

private:
    void DrawDashboard(AssetManager& assetManager, std::vector<Pawn*>& worldPawns, std::vector<std::unique_ptr<Pawn>>& ownedPawns);

    void DrawPawnPanel(std::vector<Pawn*>& worldPawns, std::vector<std::unique_ptr<Pawn>>& ownedPawns);

    void DrawAssetPanel(AssetManager& assetManager, std::vector<Pawn*>& worldPawns);

    void ApplyStyle();

    void RemovePawnFromWorld(size_t index, std::vector<Pawn*>& worldPawns, std::vector<std::unique_ptr<Pawn>>& ownedPawns);

    int m_selectedPawnIndex = 0;

    bool m_initialized = false;
};
