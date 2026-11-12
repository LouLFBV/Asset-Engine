#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>


using Job = std::function<void()>;


struct JobSystem {
	explicit JobSystem(int threads = 0);
	~JobSystem();
	void Submit(Job job, bool urgent = false); // urgent : passe devant
	void WaitIdle(); // attendre la fin de tout
	void WorkerLoop();
	std::vector<std::thread> jb_workers;
	std::deque<Job> jb_queue; // jobs en attente
	std::mutex jb_mutex;
	std::condition_variable jb_cvWork, jb_cvIdle;
	int jb_pending = 0; // soumis, pas terminés
	bool jb_stop = false;
};