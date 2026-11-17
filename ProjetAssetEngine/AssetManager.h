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
    std::unordered_map<Guid, Asset*> m_assetRegistry;

    // Le conteneur secret pour ne pas faire lagger le jeu !
    // Il associe l'UUID de l'asset à sa tâche de chargement en arrière-plan.
    std::set<Guid> m_loadingTasks;

public:
    AssetManager();
    ~AssetManager() = default;
    void LoadSync(Asset* asset);

    //ASYNCHRONE (Ne bloque pas le jeu)
    void LoadAsync(Asset* asset);

    void ScanDirectory(const std::string& directoryPath);

    JobSystem& GetJobSystem();

    std::unordered_map<Guid, Asset*>& GetRegistry() { return m_assetRegistry; };
};