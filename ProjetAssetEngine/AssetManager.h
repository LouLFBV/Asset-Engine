#pragma once
#include <set>
#include <xhash>
#include <chrono>
#include "Asset.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "RTTI.h"
#include "JobSystem.h"

struct Guid;

class AssetManager {
private:
    JobSystem m_assetJobSystem;
    std::unordered_map<Guid, Asset*> m_assetTree;

    // Le conteneur secret pour ne pas faire lagger le jeu !
    // Il associe l'UUID de l'asset à sa tâche de chargement en arrière-plan.
    std::set<Guid> m_loadingTasks;

public:
    AssetManager();
    ~AssetManager() = default;
    void LoadSync(Asset* asset);

    //ASYNCHRONE (Ne bloque pas le jeu)
    void LoadAsync(Asset* asset);

    JobSystem& GetJobSystem();
};