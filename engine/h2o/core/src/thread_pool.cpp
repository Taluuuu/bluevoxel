#include "core/thread_pool.h"

#include "core/types.h"

namespace h2o
{
    void ThreadPool::start()
    {
        m_should_terminate = false;
        m_threads.clear();

        const i32 num_threads = 4;//i32(std::thread::hardware_concurrency());//std::min(4, i32(std::thread::hardware_concurrency()));
        for (i32 i = 0; i < num_threads; ++i)
            m_threads.emplace_back(&ThreadPool::thread_loop, this);
    }

    void ThreadPool::stop()
    {
        {
            std::unique_lock lock(m_queue_mutex);
            m_should_terminate = true;
        }

        m_mutex_condition.notify_all();
        for (auto& thread : m_threads)
            thread.join();

        m_threads.clear();
    }

    void ThreadPool::queue_job(f32 priority, const Job& job)
    {
        {
            std::unique_lock lock(m_queue_mutex);
            m_job_queue.insert({ priority, job });
        }

        m_mutex_condition.notify_one();
    }

    void ThreadPool::thread_loop()
    {
        while (true)
        {
            Job job;
            {
                std::unique_lock lock(m_queue_mutex);
                m_mutex_condition.wait(lock,
                    [this]() -> bool
                    {
                        return !m_job_queue.empty() || m_should_terminate;
                    }
                );

                if (m_should_terminate)
                    return;

                job = m_job_queue.begin()->second;
                m_job_queue.erase(m_job_queue.begin());
            }

            job();
        }
    }
}