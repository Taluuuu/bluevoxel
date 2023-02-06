#pragma once

#include "types.h"

namespace engine
{
    class IWindowModule
    {
    public:

        virtual ~IWindowModule() {}

        /**
         * @brief Get the time since the last frame in seconds
         * 
         * @return The time since the last frame in seconds
         */
        virtual f64 delta_time() const = 0;

        /**
         * @brief Getter for if the window should close
         * 
         * @return true if the window should close
         */
        virtual bool should_close() const = 0;

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
        virtual void swap_buffers(f64 max_fps = 60.0) const = 0;

    };

    class ITickable
    {
    public:

        virtual ~ITickable() {}

        /**
         * @brief Called every frame
         * 
         * @param delta_time The time since the last frame in seconds
         */
        virtual void tick(f64 delta_time) = 0;

    };
}