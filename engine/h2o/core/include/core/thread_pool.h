#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
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

        void queue_job(const Job& job);

    private:

        // This is run on every thread; each thread loops until they get
        // a new job to run.
        void thread_loop();

    private:

        bool m_should_terminate = false;

        std::mutex m_queue_mutex{};
        std::condition_variable m_mutex_condition{};
        std::queue<Job> m_job_queue{};

        std::vector<std::thread> m_threads{};

    };
}