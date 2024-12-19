#include "core/profiling/scope_timer.h"

#include "core/log.h"
#include "core/types.h"

namespace h2o
{
    ScopeTimer::ScopeTimer(const char* name)
        : m_name(name)
        , m_start_time(std::chrono::high_resolution_clock::now())
    {}

    ScopeTimer::~ScopeTimer()
    {
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - m_start_time);

        const f32 duration_ms = duration.count() / 1000.0f;
        log::info("Scope Timer '{}' : {}ms", m_name, duration_ms);
    }
}
