#include "window_glfw.h"

#include <glfw/glfw3.h>

namespace engine
{
    Window_GLFW::Window_GLFW(GLFWwindow* window_handle)
        : m_handle(window_handle)
    {
        
    }

    Window_GLFW::Window_GLFW(Window_GLFW&& other)
    {
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    Window_GLFW::~Window_GLFW()
    {
        if (m_handle)
            glfwDestroyWindow(m_handle);
    }

    v2i Window_GLFW::size() const
    {
        i32 width, height;
        glfwGetWindowSize(m_handle, &width, &height);
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
        glfwPollEvents();
    }

    void Window_GLFW::swap_buffers(f64 max_fps)
    {
        glfwSwapBuffers(m_handle);

        f64 curTime = glfwGetTime();

        // Apply framerate lock
        f64 targetTime = m_previous_time + (1.0 / max_fps);
        while(curTime < targetTime)
            curTime = glfwGetTime();
        
        m_delta_time = curTime - m_previous_time;
        m_previous_time = curTime;
    }

    const char** Window_GLFW::vulkan_instance_extensions(u32& count) const
    {
        return glfwGetRequiredInstanceExtensions(&count);
    }
}