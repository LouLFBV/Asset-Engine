
#pragma once

#include <set>
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>

#include "Asset.h"
#include "Guid.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "RTTI.h"
#include "JobSystem.h"
#include "Pawn.h"

class AssetManager
{
private:

    JobSystem m_assetJobSystem;

    std::unordered_map<Guid, Asset*> m_assetRegistry;

    // Assets actuellement en cours de chargement.
    std::set<Guid> m_loadingTasks;

    // Protection du set.
    std::mutex m_loadingMutex;

    // Destruction des assets.
    void ClearRegistry();

public:

    AssetManager();

    ~AssetManager();

    void LoadSync(Asset* asset);

    void LoadAsync(Asset* asset);

    void ScanDirectory(const std::string& directoryPath, std::vector<Pawn*>& worldPawns);

    JobSystem& GetJobSystem();

    std::unordered_map<Guid, Asset*>& GetRegistry()
    {
        return m_assetRegistry;
    }
};
