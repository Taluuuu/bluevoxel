#pragma once

#include "core/types.h"

namespace engine
{
    class IWindow
    {
    public:

        virtual ~IWindow() {}

        /**
         * @brief Get the size of the window in pixels
         * 
         * @return The size of the window in pixels
         */
        virtual v2i size() const = 0;

        /**
         * @brief Getter for if the window should stay open
         * 
         * @return true if the window should stay open
         */
        virtual bool should_close() const = 0;

        /**
         * @brief Getter for the time between the last two frames in seconds
         * 
         * @return The time between the last two frames in seconds
         */
        virtual f64 delta_time() const = 0;

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
        virtual void swap_buffers(f64 max_fps = 60.0) = 0;
        
    };
}