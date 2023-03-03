#pragma once

#include "core/core_interfaces.h"
#include "core/events.h"
#include "core/types.h"

namespace h2o
{
    struct WindowResizeEvent
    {
        v2u new_size{};
    };

    class IWindow
    {
    public:

        virtual ~IWindow() = default;

        /**
         * @brief Get the size of the window in screen coordinates
         * 
         * @return The size of the window in screen coordinates
         */
        [[nodiscard]] virtual v2i window_size() const = 0;

        /**
         * @brief Get the size of the window in pixels
         * 
         * @return The size of the window in pixels
         */
        [[nodiscard]] virtual v2i framebuffer_size() const = 0;

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
         * @brief Get this window's wrapped instance pointer. With a GLFW
         *        window, this is the GLFWwindow* pointer.
         * 
         * @return void* 
         */
        [[nodiscard]] virtual void* handle() const = 0;

        /**
         * @brief Swap the window's buffers
         * 
         * @param max_fps The fps lock
         */
        virtual void swap_buffers(f64 max_fps) = 0;

        virtual Event<WindowResizeEvent>& resize_event() = 0;
        
    };
}