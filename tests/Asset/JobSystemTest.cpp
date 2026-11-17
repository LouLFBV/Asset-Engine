#include <AssetEngine/Asset/JobSystem.h>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>
#include <vector>

using namespace AssetEngine;

TEST_CASE("1000 jobs run once each")
{
	std::vector<std::atomic<int>> runs(1000);

	JobSystem jobs(4);
	for (std::atomic<int>& run : runs)
		jobs.Submit([&run] { ++run; });

	jobs.WaitIdle();

	for (const std::atomic<int>& run : runs)
		REQUIRE(run == 1);
}

TEST_CASE("Jobs run on other threads than the main one")
{
	JobSystem jobs(2);
	REQUIRE(jobs.GetThreadCount() == 2);

	std::thread::id jobThread;
	jobs.Submit([&jobThread] { jobThread = std::this_thread::get_id(); });
	jobs.WaitIdle();

	REQUIRE(jobThread != std::thread::id{});
	REQUIRE(jobThread != std::this_thread::get_id());
}

TEST_CASE("The pool finishes its jobs before stopping")
{
	std::atomic<int> done = 0;
	{
		JobSystem jobs(2);
		for (int i = 0; i < 50; ++i)
		{
			jobs.Submit([&done] {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				++done;
			});
		}

		// No WaitIdle: the destructor must still run every job, then join the threads.
	}

	REQUIRE(done == 50);
}

TEST_CASE("A job can submit other jobs")
{
	std::atomic<int> done = 0;

	JobSystem jobs(2);
	jobs.Submit([&] {
		for (int i = 0; i < 10; ++i)
			jobs.Submit([&done] { ++done; });
	});

	jobs.WaitIdle();
	REQUIRE(done == 10);
}

TEST_CASE("The default pool has at least one thread")
{
	JobSystem jobs;
	REQUIRE(jobs.GetThreadCount() >= 1);
}
