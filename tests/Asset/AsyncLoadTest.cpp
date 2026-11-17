#include "AssetTestUtils.h"

#include <AssetEngine/Asset/AssetManager.h>
#include <AssetEngine/Asset/JobSystem.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <future>
#include <string>
#include <thread>
#include <vector>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

namespace
{
	// Counts how many times its file is read.
	class Level : public Asset
	{
		ASSET_REFLECT(Level, 1, Asset)

		void Serialize(Archive& archive) override
		{
			if (archive.IsLoading())
				++reads;

			archive.Value("name", name);
		}

		std::string name;

		static inline std::atomic<int> reads = 0;
	};

	// A folder with 20 levels, already scanned.
	struct Fixture
	{
		Fixture()
		{
			for (int i = 0; i < 20; ++i)
			{
				Level level;
				level.name = "level" + std::to_string(i);
				WriteAssetFile(folder.Path() / ("levels/" + level.name + ".asset"), level);
			}

			registry.Scan();
			Level::reads = 0;
		}

		TempDirectory folder;
		AssetRegistry registry{folder.Path()};
	};

	// Keeps the only thread of a pool busy until Release is called, so that the next jobs stay in the queue.
	class Blocker
	{
	public:
		explicit Blocker(JobSystem& jobs)
		{
			jobs.Submit([this] { m_release.get_future().wait(); });
		}

		void Release() { m_release.set_value(); }

	private:
		std::promise<void> m_release;
	};
}

ASSET_REGISTER_TYPE(Level)

TEST_CASE("The callback runs in Update, on the main thread")
{
	Fixture fixture;
	JobSystem jobs(2);
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum(), &jobs);

	bool called = false;
	std::thread::id callbackThread;
	AssetHandle<Asset> result;

	manager.LoadAsync("levels/level3.asset", [&](AssetHandle<Asset> handle) {
		called = true;
		callbackThread = std::this_thread::get_id();
		result = handle;
	});

	jobs.WaitIdle();

	// The file is read, but the callback waits for Update.
	REQUIRE_FALSE(called);
	REQUIRE(manager.Update() == 1);

	REQUIRE(called);
	REQUIRE(callbackThread == std::this_thread::get_id());
	REQUIRE(result);
	REQUIRE(manager.Update() == 0);
}

TEST_CASE("Asking several times for a loading asset reads its file once")
{
	Fixture fixture;
	JobSystem jobs(1);
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum(), &jobs);
	const AssetId id = AssetIdFromPath("levels/level0.asset");

	// The only thread is busy: the load stays in the queue, and the asset stays "loading".
	Blocker blocker(jobs);

	std::vector<const Asset*> received;
	for (int i = 0; i < 10; ++i)
		manager.LoadAsync(id, [&](AssetHandle<Asset> handle) { received.push_back(handle.Get()); });

	REQUIRE(manager.IsLoading(id));
	REQUIRE_FALSE(manager.IsLoaded(id));

	blocker.Release();
	jobs.WaitIdle();
	REQUIRE(manager.Update() == 10);

	REQUIRE(Level::reads == 1);
	REQUIRE(received.size() == 10);
	for (const Asset* asset : received)
		REQUIRE(asset == received.front());
}

TEST_CASE("A synchronous load waits for the background load of the same asset")
{
	Fixture fixture;
	JobSystem jobs(1);
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum(), &jobs);
	const AssetId id = AssetIdFromPath("levels/level0.asset");

	Blocker blocker(jobs);

	AssetHandle<Asset> fromCallback;
	manager.LoadAsync(id, [&](AssetHandle<Asset> handle) { fromCallback = handle; });

	// Released a little later, from another thread: Load must wait for the job instead of reading the file.
	std::thread releaser([&] {
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		blocker.Release();
	});

	const AssetHandle<Asset> fromLoad = manager.Load(id);
	releaser.join();

	manager.Update();
	REQUIRE(fromLoad);
	REQUIRE(fromLoad.Get() == fromCallback.Get());
	REQUIRE(Level::reads == 1);
}

TEST_CASE("Many assets load in parallel")
{
	Fixture fixture;
	JobSystem jobs(4);
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum(), &jobs);

	std::vector<AssetHandle<Level>> levels;
	for (int i = 0; i < 20; ++i)
	{
		manager.LoadAsync<Level>("levels/level" + std::to_string(i) + ".asset",
			[&](AssetHandle<Level> handle) { levels.push_back(handle); });
	}

	jobs.WaitIdle();
	REQUIRE(manager.Update() == 20);

	REQUIRE(levels.size() == 20);
	for (const AssetHandle<Level>& level : levels)
		REQUIRE(level);

	REQUIRE(manager.GetLoadedCount() == 20);
	REQUIRE(Level::reads == 20);
}

TEST_CASE("An asynchronous load reports its errors in the callback")
{
	Fixture fixture;
	JobSystem jobs(2);
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum(), &jobs);

	AssetError error = AssetError::None;
	manager.LoadAsync("missing.asset", [&](AssetHandle<Asset> handle) { error = handle.GetError(); });

	jobs.WaitIdle();
	manager.Update();
	REQUIRE(error == AssetError::NotInRegistry);
}

TEST_CASE("Without a JobSystem, LoadAsync still calls back in Update")
{
	Fixture fixture;
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum());

	bool called = false;
	manager.LoadAsync<Level>("levels/level1.asset", [&](AssetHandle<Level> handle) {
		called = true;
		REQUIRE(handle->name == "level1");
	});

	REQUIRE_FALSE(called);
	REQUIRE(manager.Update() == 1);
	REQUIRE(called);
}
