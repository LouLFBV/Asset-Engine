#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace AssetEngine
{
	// A pool of threads that run jobs in the background, in the order they were submitted.
	class JobSystem
	{
	public:
		using Job = std::function<void()>;

		// 0 means one thread per core, minus the main thread.
		explicit JobSystem(std::size_t threadCount = 0);

		// Runs every job already submitted, then stops the threads.
		~JobSystem();

		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		// Can be called from any thread, including from a job.
		void Submit(Job job);

		// Blocks until every submitted job is done.
		// Main thread only: a job that waits for the pool could wait for itself forever.
		void WaitIdle();

		std::size_t GetThreadCount() const { return m_threads.size(); }

	private:
		void WorkerLoop();

		std::mutex m_mutex;
		std::condition_variable m_jobAvailable;
		std::condition_variable m_idle;
		std::queue<Job> m_jobs;
		std::size_t m_running = 0;
		bool m_stopping = false;

		// Declared last: the threads start once everything they use exists.
		std::vector<std::thread> m_threads;
	};
}
