#include <AssetEngine/Asset/AssetManager.h>

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/FileInputStream.h>

#include <chrono>
#include <utility>

namespace AssetEngine
{
	namespace
	{
		bool IsReady(const std::shared_future<AssetHandle<Asset>>& load)
		{
			return load.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
	}

	AssetManager::AssetManager(const AssetRegistry& registry, SecurityPolicy policy, JobSystem* jobs)
		: m_registry(registry)
		, m_policy(std::move(policy))
		, m_jobs(jobs)
	{
	}

	AssetManager::~AssetManager()
	{
		std::vector<PendingLoad> running;
		{
			std::lock_guard lock(m_mutex);
			for (const auto& [id, entry] : m_entries)
			{
				if (entry.pending.valid())
					running.push_back(entry.pending);
			}
		}

		// Waited without the lock: the jobs need it to finish.
		for (const PendingLoad& load : running)
			load.wait();
	}

	AssetHandle<Asset> AssetManager::Load(AssetId id)
	{
		PendingLoad pending;
		std::promise<AssetHandle<Asset>> promise;
		{
			std::lock_guard lock(m_mutex);
			Entry& entry = m_entries[id];

			// 1. Already in memory: everyone shares the same instance.
			if (std::shared_ptr<Asset> asset = entry.asset.lock())
				return {id, std::move(asset)};

			// 2. Being loaded by someone else: wait for that load instead of reading the file twice.
			if (entry.pending.valid())
				pending = entry.pending;
			else
				entry.pending = promise.get_future().share();
		}

		if (pending.valid())
			return pending.get();

		// 3. Nobody loads it: this thread reads the file, and the others wait for it.
		const AssetHandle<Asset> handle = ReadFromDisk(id);
		Finish(id, handle, promise);
		return handle;
	}

	void AssetManager::LoadAsync(AssetId id, LoadCallback callback)
	{
		auto promise = std::make_shared<std::promise<AssetHandle<Asset>>>();
		{
			std::lock_guard lock(m_mutex);
			Entry& entry = m_entries[id];

			// Already in memory: the callback still waits for Update, so it always runs at the same moment.
			if (std::shared_ptr<Asset> asset = entry.asset.lock())
			{
				promise->set_value({id, std::move(asset)});
				m_waiting.push_back({promise->get_future().share(), std::move(callback)});
				return;
			}

			// Already loading: one more callback for the same load.
			if (entry.pending.valid())
			{
				m_waiting.push_back({entry.pending, std::move(callback)});
				return;
			}

			// The entry is marked as loading before the job starts, so a second request cannot start another one.
			entry.pending = promise->get_future().share();
			m_waiting.push_back({entry.pending, std::move(callback)});
		}

		auto job = [this, id, promise] {
			const AssetHandle<Asset> handle = ReadFromDisk(id);
			Finish(id, handle, *promise);
		};

		if (m_jobs != nullptr)
			m_jobs->Submit(std::move(job));
		else
			job();
	}

	std::size_t AssetManager::Update()
	{
		std::vector<Waiting> ready;
		{
			std::lock_guard lock(m_mutex);
			for (auto waiting = m_waiting.begin(); waiting != m_waiting.end();)
			{
				if (IsReady(waiting->load))
				{
					ready.push_back(std::move(*waiting));
					waiting = m_waiting.erase(waiting);
				}
				else
				{
					++waiting;
				}
			}
		}

		// Called without the lock: a callback may call Load or LoadAsync.
		for (Waiting& waiting : ready)
			waiting.callback(waiting.load.get());

		return ready.size();
	}

	bool AssetManager::IsLoaded(AssetId id) const
	{
		std::lock_guard lock(m_mutex);
		const auto entry = m_entries.find(id);
		return entry != m_entries.end() && !entry->second.asset.expired();
	}

	bool AssetManager::IsLoading(AssetId id) const
	{
		std::lock_guard lock(m_mutex);
		const auto entry = m_entries.find(id);
		return entry != m_entries.end() && entry->second.pending.valid();
	}

	std::size_t AssetManager::GetLoadedCount() const
	{
		std::lock_guard lock(m_mutex);

		std::size_t count = 0;
		for (const auto& [id, entry] : m_entries)
		{
			if (!entry.asset.expired())
				++count;
		}

		return count;
	}

	AssetHandle<Asset> AssetManager::ReadFromDisk(AssetId id) const
	{
		const AssetRecord* record = m_registry.Find(id);
		if (record == nullptr)
			return {id, nullptr, AssetError::NotInRegistry};

		FileInputStream stream(m_registry.GetFullPath(*record));
		if (!stream.IsOpen())
			return {id, nullptr, AssetError::FileNotFound};

		AssetLoadResult result = ReadAsset(stream, m_policy);
		if (result.error != AssetError::None)
			return {id, nullptr, result.error};

		return {id, std::shared_ptr<Asset>(std::move(result.asset))};
	}

	void AssetManager::Finish(AssetId id, const AssetHandle<Asset>& handle, std::promise<AssetHandle<Asset>>& promise)
	{
		std::lock_guard lock(m_mutex);
		Entry& entry = m_entries[id];
		entry.asset = handle.GetShared();
		entry.pending = {};

		// Still under the lock: once the load is no longer pending, the job touches nothing of the manager,
		// so the destructor can never free the manager while a job still uses it.
		promise.set_value(handle);
	}
}
