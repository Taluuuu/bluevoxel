#pragma once

#include "core/types.h"
#include "windowing/window.h"

#include <string>

struct GLFWwindow;

namespace engine
{
    class Window_GLFW : public IWindow
    {
    public:

        Window_GLFW(const std::string_view& title, v2i size);
        Window_GLFW(const Window_GLFW&) = delete;
        Window_GLFW(Window_GLFW&& other);
        ~Window_GLFW();

    public:

        // IWindow interface
        virtual v2i size() const override;
        virtual bool should_close() const override;
        virtual f64 delta_time() const override;
        virtual void poll_events() const override;
        virtual void* wrapped_window_handle() const override;
        virtual void swap_buffers(f64 max_fps = 60.0) override;

    private:
        
        GLFWwindow* m_handle = nullptr;

        f64 m_previous_time = 0.0f;
        f64 m_delta_time = 0.0f;

    };
}