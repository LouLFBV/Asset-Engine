#include "JobSystem.h"
#include <iostream>

JobSystem::JobSystem(int threads)
{
	if (threads == 0) threads = std::thread::hardware_concurrency();

	for (size_t i = 0; i < threads; ++i)
		jb_workers.emplace_back([this] {WorkerLoop(); });
}

JobSystem::~JobSystem()
{
	{
		std::lock_guard<std::mutex> lock(jb_mutex);
		jb_stop = true;
	}
	jb_cvWork.notify_all();

	for (std::thread& t : jb_workers)
		t.join();
}

void JobSystem::Submit(Job job, bool urgent)
{
	{
		std::lock_guard<std::mutex> lock(jb_mutex);
		if (urgent)
			jb_queue.emplace_front(std::move(job));
		else
			jb_queue.emplace_back(std::move(job));
		jb_pending++;
	}
	jb_cvWork.notify_one();
}

void JobSystem::WaitIdle()
{
	std::unique_lock<std::mutex> lock(jb_mutex);
	jb_cvIdle.wait(lock, [&] {
		return jb_pending == 0;
		});
}

void JobSystem::WorkerLoop() {
	for (;;) {
		Job job;
		{
			std::unique_lock<std::mutex> lock(jb_mutex);
			jb_cvWork.wait(lock, [&] {
				return jb_stop || !jb_queue.empty();
				});
			if (jb_queue.empty()) return;
			job = std::move(jb_queue.front());
			jb_queue.pop_front();
		}
		job();
		std::lock_guard<std::mutex> lock(jb_mutex);
		if (--jb_pending == 0) jb_cvIdle.notify_all(); // Tout est fini
	}
}