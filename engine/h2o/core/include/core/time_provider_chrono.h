#pragma once

#include <chrono>

#include "core_interfaces.h"
#include "tickable.h"

namespace h2o
{
    class TimeProvider_Chrono
        : public Tickable
        , public ITimeProvider
    {
    public:

        TimeProvider_Chrono();

        // ITimeProvider interface
        [[nodiscard]] f64 time() const override;
        [[nodiscard]] f64 delta_time() const override;

    protected:

        // Tickable interface
        void frame_start() override;

    private:

        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        TimePoint m_first_update_time{};
        TimePoint m_previous_update_time{};

        f64 m_delta_time = 0.0f;
        f64 m_time = 0.0f;

    };
}
