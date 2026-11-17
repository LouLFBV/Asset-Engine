#pragma once

#include <AssetEngine/Asset/Asset.h>
#include <AssetEngine/Asset/AssetHandle.h>
#include <AssetEngine/Asset/AssetId.h>
#include <AssetEngine/Asset/AssetRegistry.h>
#include <AssetEngine/Asset/JobSystem.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <concepts>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace AssetEngine
{
	// Loads the assets of a registry, synchronously or in the background,
	// and shares each loaded asset between all its users.
	// The registry must not be scanned again while assets are loading.
	class AssetManager
	{
	public:
		using LoadCallback = std::function<void(AssetHandle<Asset>)>;

		// Without a JobSystem, LoadAsync reads the file at once, but still calls back in Update.
		AssetManager(const AssetRegistry& registry, SecurityPolicy policy, JobSystem* jobs = nullptr);

		// Waits for the loads still running: they use this manager.
		~AssetManager();

		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		// Returns the asset already in memory, waits for it if it is loading, or reads its file.
		AssetHandle<Asset> Load(AssetId id);
		AssetHandle<Asset> Load(std::string_view path) { return Load(AssetIdFromPath(path)); }

		template <std::derived_from<Asset> T>
		AssetHandle<T> Load(AssetId id)
		{
			return Cast<T>(Load(id));
		}

		template <std::derived_from<Asset> T>
		AssetHandle<T> Load(std::string_view path)
		{
			return Load<T>(AssetIdFromPath(path));
		}

		// Starts loading in the background, and returns at once.
		// The callback runs later, on the thread that calls Update.
		void LoadAsync(AssetId id, LoadCallback callback);
		void LoadAsync(std::string_view path, LoadCallback callback) { LoadAsync(AssetIdFromPath(path), std::move(callback)); }

		template <std::derived_from<Asset> T, std::invocable<AssetHandle<T>> Callback>
		void LoadAsync(AssetId id, Callback callback)
		{
			LoadAsync(id, [callback = std::move(callback)](AssetHandle<Asset> handle) mutable {
				callback(Cast<T>(handle));
			});
		}

		template <std::derived_from<Asset> T, std::invocable<AssetHandle<T>> Callback>
		void LoadAsync(std::string_view path, Callback callback)
		{
			LoadAsync<T>(AssetIdFromPath(path), std::move(callback));
		}

		// Calls the callbacks of the finished loads. Call it once per frame, on the main thread.
		// Returns the number of callbacks called.
		std::size_t Update();

		bool IsLoaded(AssetId id) const;
		bool IsLoading(AssetId id) const;
		std::size_t GetLoadedCount() const;

	private:
		using PendingLoad = std::shared_future<AssetHandle<Asset>>;

		struct Entry
		{
			// weak_ptr: the cache finds an asset in memory, but does not keep it alive.
			std::weak_ptr<Asset> asset;

			// Valid while the file is being read: everyone who asks for the asset waits for this load.
			PendingLoad pending;
		};

		struct Waiting
		{
			PendingLoad load;
			LoadCallback callback;
		};

		template <typename T>
		static AssetHandle<T> Cast(const AssetHandle<Asset>& handle)
		{
			if (!handle)
				return {handle.GetId(), nullptr, handle.GetError()};

			std::shared_ptr<T> typed = std::dynamic_pointer_cast<T>(handle.GetShared());
			if (!typed)
				return {handle.GetId(), nullptr, AssetError::WrongType};

			return {handle.GetId(), std::move(typed)};
		}

		// Reads the file. Runs on any thread: it only reads the registry and the policy.
		AssetHandle<Asset> ReadFromDisk(AssetId id) const;

		// Stores the result of a load, and wakes up everyone who waits for it.
		void Finish(AssetId id, const AssetHandle<Asset>& handle, std::promise<AssetHandle<Asset>>& promise);

		const AssetRegistry& m_registry;
		SecurityPolicy m_policy;
		JobSystem* m_jobs = nullptr;

		// m_entries and m_waiting are used by the jobs and by the main thread: always with the mutex.
		mutable std::mutex m_mutex;
		std::unordered_map<AssetId, Entry> m_entries;
		std::vector<Waiting> m_waiting;
	};
}
