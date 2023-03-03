#pragma once

#include "core/types.h"
#include "windowing/window.h"

#include <memory>
#include <string>

struct GLFWwindow;

namespace h2o
{
    class Window_GLFW : public IWindow
    {
    public:

        static std::shared_ptr<Window_GLFW> create(const std::string_view& title, v2i size);

    private:

        explicit Window_GLFW(GLFWwindow* window);

    public:

        Window_GLFW(const Window_GLFW&) = delete;
        Window_GLFW(Window_GLFW&& other) noexcept;
        ~Window_GLFW() override;

    public:

        // IWindow interface
        [[nodiscard]] v2i window_size() const override;
        [[nodiscard]] v2i framebuffer_size() const override;
        [[nodiscard]] bool should_close() const override;
        [[nodiscard]] f64 delta_time() const override;
        void poll_events() const override;
        [[nodiscard]] void* handle() const override;
        void swap_buffers(f64 max_fps) override;
        Event<WindowResizeEvent>& resize_event() override;

    private:

        static void framebuffer_size_callback(GLFWwindow* window_handle, int width, int height);

    private:
        
        GLFWwindow* m_handle = nullptr;

        Event<WindowResizeEvent> m_resize_event;

        f64 m_previous_time = 0.0f;
        f64 m_delta_time = 0.0f;

    };
}