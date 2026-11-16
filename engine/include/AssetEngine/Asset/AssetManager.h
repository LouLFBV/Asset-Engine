#pragma once

#include <AssetEngine/Asset/Asset.h>
#include <AssetEngine/Asset/AssetHandle.h>
#include <AssetEngine/Asset/AssetId.h>
#include <AssetEngine/Asset/AssetRegistry.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <concepts>
#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace AssetEngine
{
	// Loads the assets of a registry, and shares each loaded asset between all its users.
	class AssetManager
	{
	public:
		AssetManager(const AssetRegistry& registry, SecurityPolicy policy);

		// Returns the asset already in memory, or reads its file.
		AssetHandle<Asset> Load(AssetId id);
		AssetHandle<Asset> Load(std::string_view path) { return Load(AssetIdFromPath(path)); }

		// The same, with the asset type checked: WrongType if the asset is not a T.
		template <std::derived_from<Asset> T>
		AssetHandle<T> Load(AssetId id)
		{
			const AssetHandle<Asset> handle = Load(id);
			if (!handle)
				return {id, nullptr, handle.GetError()};

			std::shared_ptr<T> typed = std::dynamic_pointer_cast<T>(handle.GetShared());
			if (!typed)
				return {id, nullptr, AssetError::WrongType};

			return {id, std::move(typed)};
		}

		template <std::derived_from<Asset> T>
		AssetHandle<T> Load(std::string_view path)
		{
			return Load<T>(AssetIdFromPath(path));
		}

		// True while at least one handle keeps the asset in memory.
		bool IsLoaded(AssetId id) const;
		std::size_t GetLoadedCount() const;

	private:
		const AssetRegistry& m_registry;
		SecurityPolicy m_policy;

		// weak_ptr: the cache finds an asset in memory, but does not keep it alive.
		std::unordered_map<AssetId, std::weak_ptr<Asset>> m_cache;
	};
}
