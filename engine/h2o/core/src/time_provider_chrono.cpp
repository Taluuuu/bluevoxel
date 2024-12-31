#include "core/time_provider_chrono.h"

#include "core/engine.h"

namespace h2o
{
    TimeProvider_Chrono::TimeProvider_Chrono()
        : Tickable(g_engine)
    {
        set_tick_phases(TickPhase::FrameStart);

        m_first_update_time = Clock::now();
    }

    f64 TimeProvider_Chrono::time() const
    {
        return m_time;
    }

    f64 TimeProvider_Chrono::delta_time() const
    {
        return m_delta_time;
    }

    void TimeProvider_Chrono::frame_start()
    {
        const auto last_update = m_previous_update_time;
        m_previous_update_time = Clock::now();

        {
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                m_previous_update_time - last_update);

            m_delta_time = static_cast<f64>(duration.count()) / 1'000'000.0;
        }

        {
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                m_previous_update_time - m_first_update_time);

            m_time = static_cast<f64>(duration.count()) / 1'000'000.0;
        }
    }
}
