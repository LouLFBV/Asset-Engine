
#include "JobSystem.h"

#include <iostream>
#include <stdexcept>

JobSystem::JobSystem(int threads)
{
    if (threads <= 0)
        threads = static_cast<int>(std::thread::hardware_concurrency());

    if (threads <= 0)
        threads = 1;

    for (std::size_t i = 0; i < threads; ++i)
    {
        jb_workers.emplace_back([this]()
            {
                WorkerLoop();
            });
    }
}

JobSystem::~JobSystem()
{
    WaitIdle();

    {
        std::lock_guard<std::mutex> lock(jb_mutex);
        jb_stop = true;
    }

    jb_cvWork.notify_all();

    for (std::thread& worker : jb_workers)
    {
        if (worker.joinable())
            worker.join();
    }
}

void JobSystem::Submit(Job job, bool urgent)
{
    if (!job)
        return;

    {
        std::lock_guard<std::mutex> lock(jb_mutex);

        if (jb_stop)
        {
            throw std::runtime_error(
                "Impossible de soumettre une tache : JobSystem arrete."
            );
        }

        if (urgent)
            jb_queue.emplace_front(std::move(job));
        else
            jb_queue.emplace_back(std::move(job));

        ++jb_pending;
    }

    jb_cvWork.notify_one();
}

void JobSystem::WaitIdle()
{
    std::unique_lock<std::mutex> lock(jb_mutex);

    jb_cvIdle.wait(lock, [this]()
        {
            return jb_pending == 0;
        });
}

void JobSystem::WorkerLoop()
{
    while (true)
    {
        Job job;

        {
            std::unique_lock<std::mutex> lock(jb_mutex);

            jb_cvWork.wait(lock, [this]()
                {
                    return jb_stop || !jb_queue.empty();
                });

            if (jb_queue.empty() && jb_stop)
                return;

            job = std::move(jb_queue.front());
            jb_queue.pop_front();
        }

        try
        {
            job();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Erreur dans un Job : "
                << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Erreur inconnue dans un Job."
                << std::endl;
        }

        {
            std::lock_guard<std::mutex> lock(jb_mutex);

            --jb_pending;

            if (jb_pending == 0)
                jb_cvIdle.notify_all();
        }
    }
}
