#include <AssetEngine/Asset/JobSystem.h>

#include <algorithm>
#include <utility>

namespace AssetEngine
{
	JobSystem::JobSystem(std::size_t threadCount)
	{
		if (threadCount == 0)
		{
			// hardware_concurrency may return 0 when it does not know: at least one thread.
			const std::size_t cores = std::thread::hardware_concurrency();
			threadCount = std::max<std::size_t>(1, cores > 1 ? cores - 1 : 1);
		}

		for (std::size_t i = 0; i < threadCount; ++i)
			m_threads.emplace_back(&JobSystem::WorkerLoop, this);
	}

	JobSystem::~JobSystem()
	{
		{
			std::lock_guard lock(m_mutex);
			m_stopping = true;
		}

		m_jobAvailable.notify_all();
		for (std::thread& thread : m_threads)
			thread.join();
	}

	void JobSystem::Submit(Job job)
	{
		{
			std::lock_guard lock(m_mutex);
			m_jobs.push(std::move(job));
		}

		m_jobAvailable.notify_one();
	}

	void JobSystem::WaitIdle()
	{
		std::unique_lock lock(m_mutex);
		m_idle.wait(lock, [this] { return m_jobs.empty() && m_running == 0; });
	}

	void JobSystem::WorkerLoop()
	{
		for (;;)
		{
			Job job;
			{
				std::unique_lock lock(m_mutex);

				// Sleeps without using the CPU until there is a job, or until the pool stops.
				m_jobAvailable.wait(lock, [this] { return m_stopping || !m_jobs.empty(); });

				// Stopping, and nothing left to do.
				if (m_jobs.empty())
					return;

				job = std::move(m_jobs.front());
				m_jobs.pop();
				++m_running;
			}

			// The job runs without the lock, so other threads can take jobs at the same time.
			job();

			{
				std::lock_guard lock(m_mutex);
				--m_running;
				if (m_jobs.empty() && m_running == 0)
					m_idle.notify_all();
			}
		}
	}
}
