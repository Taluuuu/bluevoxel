#include "window_glfw.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace engine
{
    std::shared_ptr<Window_GLFW> Window_GLFW::create(const std::string_view& title, v2i size)
    {
#if TNT_USE_VULKAN
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#elif TNT_USE_OPENGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWwindow* window = glfwCreateWindow(size.x, size.y, title.data(), NULL, NULL);
#if TNT_USE_OPENGL
        glfwMakeContextCurrent(window);
#endif

        if (!window)
            return nullptr;

        return std::shared_ptr<Window_GLFW>(new Window_GLFW(window));
    }

    Window_GLFW::Window_GLFW(GLFWwindow* window)
        : m_handle(window)
    {
        glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);
        glfwSetWindowUserPointer(m_handle, this);
    }

    Window_GLFW::Window_GLFW(Window_GLFW&& other) noexcept
    {
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    Window_GLFW::~Window_GLFW()
    {
        assert(m_handle);
        glfwDestroyWindow(m_handle);
    }

    v2i Window_GLFW::window_size() const
    {
        i32 width, height;
        glfwGetWindowSize(m_handle, &width, &height);
        return { width, height };
    }

    v2i Window_GLFW::framebuffer_size() const
    {
        i32 width, height;
        glfwGetFramebufferSize(m_handle, &width, &height);
        return { width, height };
    }

    bool Window_GLFW::should_close() const
    {
        return glfwWindowShouldClose(m_handle);
    }

    f64 Window_GLFW::delta_time() const
    {
        return m_delta_time;
    }

    void Window_GLFW::poll_events() const
    {
        // Managing window input?
    }

    void *Window_GLFW::handle() const
    {
        return m_handle;
    }

    void Window_GLFW::swap_buffers(f64 max_fps)
    {
#if TNT_USE_OPENGL
         glfwSwapBuffers(m_handle);
#endif

        f64 curTime = glfwGetTime();

        // Apply framerate lock
        f64 targetTime = m_previous_time + (1.0 / max_fps);
        while(curTime < targetTime)
            curTime = glfwGetTime();
        
        m_delta_time = curTime - m_previous_time;
        m_previous_time = curTime;
    }

    Event<WindowResizeEvent>& Window_GLFW::resize_event()
    {
        return m_resize_event;
    }

    void Window_GLFW::framebuffer_size_callback(GLFWwindow* window_handle, int width, int height)
    {
        auto window = reinterpret_cast<Window_GLFW*>(glfwGetWindowUserPointer(window_handle));
        window->m_resize_event.broadcast({{ width, height }});
    }
}