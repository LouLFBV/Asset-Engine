#pragma once

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

struct JobSystem
{
    explicit JobSystem(int threads = 0);
    ~JobSystem();

    void Submit(Job job, bool urgent = false);
    void WaitIdle();
    void WorkerLoop();

    std::vector<std::thread> jb_workers;
    std::deque<Job> jb_queue;
    std::mutex jb_mutex;

    std::condition_variable jb_cvWork;
    std::condition_variable jb_cvIdle;

    std::size_t jb_pending = 0;
    bool jb_stop = false;
};
