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

        static Window_GLFW* create(const std::string_view& title, v2i size);

    private:

        explicit Window_GLFW(GLFWwindow* window);

    public:

        Window_GLFW(const Window_GLFW&) = delete;
        Window_GLFW(Window_GLFW&& other) noexcept;
        ~Window_GLFW() override;

        // IWindow interface
        [[nodiscard]] v2i window_size() const override;
        [[nodiscard]] v2i framebuffer_size() const override;
        [[nodiscard]] bool should_close() const override;
        [[nodiscard]] f64 delta_time() const override;
        void poll_events() const override;
        [[nodiscard]] void* handle() const override;
        void swap_buffers(f64 max_fps) override;
        void set_capture_mouse(bool capture) override;
        Event<WindowResizeEvent>& resize_event() override;
        Event<KeyChangedEvent>& key_changed_event() override;
        Event<CharEvent>& char_event() override;
        Event<MouseButtonChangedEvent>& mouse_button_changed_event() override;
        Event<MouseMovedEvent>& mouse_moved_event() override;
        
    private:

        static void key_callback(GLFWwindow* window_handle, int key, int scancode, int action, int mods);
        static void char_callback(GLFWwindow* window_handle, unsigned int codepoint);
        static void mouse_button_callback(GLFWwindow* window_handle, int button, int action, int mods);
        static void framebuffer_size_callback(GLFWwindow* window_handle, int width, int height);
        static void mouse_moved_callback(GLFWwindow* window_handle, double xpos, double ypos);

    private:
        
        GLFWwindow* m_handle = nullptr;

        Event<WindowResizeEvent> m_resize_event;
        Event<KeyChangedEvent> m_key_changed_event;
        Event<CharEvent> m_char_event;
        Event<MouseButtonChangedEvent> m_mouse_button_changed_event;
        Event<MouseMovedEvent> m_mouse_moved_event;

        f64 m_previous_time = 0.0f;
        f64 m_delta_time = 0.0f;

    };
}