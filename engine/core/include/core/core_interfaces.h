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

    class ITickable
    {
    public:

        virtual ~ITickable() = default;

        ///**
        // * @brief Called every frame
        // *
        // * @param delta_time The time since the last frame in seconds
        // */
        //virtual void tick(f64 delta_time) = 0;

        virtual void tick(f64 delta_time) { assert(false); }
        virtual bool should_tick() const  { return false;  }

    };
}