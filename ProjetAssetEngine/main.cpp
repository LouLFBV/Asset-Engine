#include <memory>
#include <vector>

#include "Window.h"
#include "ImGUIManager.h"
#include "AssetManager.h"
#include "Pawn.h"
#include "Log.h"

int main()
{
    Log::Init("engine.log");

    Window window(
        1600,
        900,
        1600,
        900,
        "Asset Engine"
    );

    if (!window.Init())
    {
        LOG_ERROR("Impossible d'initialiser SDL3 !");
        Log::Shutdown();
        return -1;
    }

    ImGUIManager imguiManager;

    if (!imguiManager.Init(
        window.GetWindow(),
        window.GetRenderer()))
    {
        LOG_ERROR("Impossible d'initialiser ImGui !");

        Log::Shutdown();
        return -1;
    }

    AssetManager assetManager;
    std::vector<Pawn*> worldPawns;

    const std::string assetsDir = ImGUIManager::ResolveAssetsDir(true);
    assetManager.ScanDirectory(assetsDir, worldPawns);

    std::vector<std::unique_ptr<Pawn>> ownedPawns;

    auto playerPawn = std::make_unique<Pawn>();

    playerPawn->health = 100;
    playerPawn->speed = 15.0f;

    playerPawn->equippedWeapon = new Weapon();

    Weapon* weapon =
        SAFE_CAST(Weapon, playerPawn->equippedWeapon);

    if (weapon)
        weapon->damage = 50;

    worldPawns.push_back(playerPawn.get());

    ownedPawns.push_back(std::move(playerPawn));

    bool isRunning = true;

    while (isRunning)
    {
        isRunning = window.PollEvents();

        if (!isRunning)
            break;

        window.Clear();

        imguiManager.BeginFrame();

        imguiManager.Render(
            window.GetRenderer(),
            assetManager,
            worldPawns,
            ownedPawns
        );

        window.Present();
    }

    assetManager.GetJobSystem().WaitIdle();

    worldPawns.clear();

    ownedPawns.clear();

    imguiManager.Shutdown();

    LOG_INFO("Moteur arrete proprement.");

    Log::Shutdown();

    return 0;
}