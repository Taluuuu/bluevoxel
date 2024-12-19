#pragma once

#include <chrono>

namespace h2o
{
    class ScopeTimer
    {
    public:

        explicit ScopeTimer(const char* name);
        ~ScopeTimer();

    private:

        const char* m_name = nullptr;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time{};

    };
}
