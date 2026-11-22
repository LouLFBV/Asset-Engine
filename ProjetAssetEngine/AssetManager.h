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

    std::set<Guid> m_loadingTasks;

    std::mutex m_loadingMutex;

    void ClearRegistry();

public:

    AssetManager();

    ~AssetManager();

    void LoadSync(Asset* asset);

    void LoadAsync(Asset* asset);

    void ScanDirectory(const std::string& directoryPath, std::vector<Pawn*>& worldPawns);

    [[nodiscard]] JobSystem& GetJobSystem() noexcept;

    [[nodiscard]] inline std::unordered_map<Guid, Asset*>& GetRegistry() noexcept
    {
        return m_assetRegistry;
    }
};
