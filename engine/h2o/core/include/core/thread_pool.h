#pragma once

#include "core/types.h"

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace h2o
{
    using Job = std::function<void()>;

    // Implementation heavily inspired from:
    // https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

    class ThreadPool
    {
    public:

        void start();
        void stop();

        /**
         * Enqueue a job.
         *
         * @param priority The job's priority; 0 is highest priority.
         * @param job The job to enqueue
         */
        void queue_job(f32 priority, const Job& job);

        [[nodiscard]] size_t job_count() const { return m_job_queue.size(); }
        [[nodiscard]] size_t thread_count() const;

    private:

        // This is run on every thread; each thread loops until they get
        // a new job to run.
        void thread_loop();

    private:

        bool m_should_terminate = false;

        std::mutex m_queue_mutex{};
        std::condition_variable m_mutex_condition{};
        std::multimap<f32, Job> m_job_queue{};

        std::vector<std::thread> m_threads{};

    };
}