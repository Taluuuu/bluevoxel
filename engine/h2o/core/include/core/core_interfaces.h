#pragma once

#include "types.h"

namespace h2o
{
    class IWindowModule
    {
    public:

        virtual ~IWindowModule() = default;

        /**
         * @brief Get the time since the last frame in seconds
         * 
         * @return The time since the last frame in seconds
         */
        [[nodiscard]] virtual f64 delta_time() const = 0;

        /**
         * @brief Poll the window for events
         * 
         */
        virtual void poll_events() const = 0;

        /**
         * @brief Swap the window's buffers
         * 
         * @param max_fps The fps lock
         */
        virtual void swap_buffers(f64 max_fps) const = 0;

    };

    class IInputModule
    {
    public:

        virtual void prepare() = 0;

    };
}