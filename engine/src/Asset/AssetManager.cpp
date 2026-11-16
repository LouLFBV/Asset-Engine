#include <AssetEngine/Asset/AssetManager.h>

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/FileInputStream.h>

#include <utility>

namespace AssetEngine
{
	AssetManager::AssetManager(const AssetRegistry& registry, SecurityPolicy policy)
		: m_registry(registry)
		, m_policy(std::move(policy))
	{
	}

	AssetHandle<Asset> AssetManager::Load(AssetId id)
	{
		// 1. Already in memory: everyone shares the same instance.
		if (const auto entry = m_cache.find(id); entry != m_cache.end())
		{
			if (std::shared_ptr<Asset> asset = entry->second.lock())
				return {id, std::move(asset)};
		}

		// 2. The registry gives the file of the asset.
		const AssetRecord* record = m_registry.Find(id);
		if (record == nullptr)
			return {id, nullptr, AssetError::NotInRegistry};

		FileInputStream stream(m_registry.GetFullPath(*record));
		if (!stream.IsOpen())
			return {id, nullptr, AssetError::FileNotFound};

		// 3. The file is verified, then read into the real type of the asset.
		AssetLoadResult result = ReadAsset(stream, m_policy);
		if (result.error != AssetError::None)
			return {id, nullptr, result.error};

		std::shared_ptr<Asset> asset = std::move(result.asset);
		m_cache[id] = asset;
		return {id, std::move(asset)};
	}

	bool AssetManager::IsLoaded(AssetId id) const
	{
		const auto entry = m_cache.find(id);
		return entry != m_cache.end() && !entry->second.expired();
	}

	std::size_t AssetManager::GetLoadedCount() const
	{
		std::size_t count = 0;
		for (const auto& [id, asset] : m_cache)
		{
			if (!asset.expired())
				++count;
		}

		return count;
	}
}
