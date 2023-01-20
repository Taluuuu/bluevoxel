#pragma once

#include "core/core_interfaces.h"
#include "core/types.h"

namespace engine
{
    class IWindow
    {
    public:

        virtual ~IWindow() {}

        /**
         * @brief Get the size of the window in screen coordinates
         * 
         * @return The size of the window in screen coordinates
         */
        virtual v2i window_size() const = 0;

        /**
         * @brief Get the size of the window in pixels
         * 
         * @return The size of the window in pixels
         */
        virtual v2i framebuffer_size() const = 0;

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
         * @brief Get this window's wrapped instance pointer. With a GLFW
         *        window, this is the GLFWwindow* pointer.
         * 
         * @return void* 
         */
        virtual void* handle() const = 0;

        /**
         * @brief Swap the window's buffers
         * 
         * @param max_fps The fps lock
         */
        virtual void swap_buffers(f64 max_fps = 60.0) = 0;
        
    };
}