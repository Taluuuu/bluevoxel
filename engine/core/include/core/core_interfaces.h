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
         * @brief Getter for if the window should close
         * 
         * @return true if the window should close
         */
        [[nodiscard]] virtual bool should_close() const = 0;

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

    enum TickPhase : u32
    {
        Input      = 1 << 0,
        PreUpdate  = 1 << 1,
        Update     = 1 << 2,
        PostUpdate = 1 << 3,
        PreRender  = 1 << 4,
        Render     = 1 << 5,
        PostRender = 1 << 6,
    };

    class ITickable
    {
    public:

        virtual ~ITickable() = default;

        virtual void tick(TickPhase phase, f64 delta_time) {}

    };
}