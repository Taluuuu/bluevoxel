#include "windowing/windowing_module.h"

#include "core/game_info.h"
#include "core/engine.h"
#include "core/log.h"
#include "window_glfw.h"

#include <GLFW/glfw3.h>

namespace h2o
{
    void WindowingModule::cleanup()
    {
        delete m_window;
        glfwTerminate();
    }

    bool WindowingModule::init(Engine& engine)
    {
        if (!glfwInit())
        {
            log::error("Failed to initialize GLFW.");
            return false;
        }

        m_window = Window_GLFW::create(engine.game_info().game_name, default_size);
        if (!m_window)
        {
            glfwTerminate();
            return false;
        }

        return true;
    }

    f64 WindowingModule::delta_time() const
    {
        assert(m_window != nullptr);
        return m_window->delta_time();
    }

    bool WindowingModule::should_close() const
    {
        assert(m_window != nullptr);
        return m_window->should_close();
    }

    void WindowingModule::poll_events() const
    {
        assert(m_window != nullptr);
        glfwPollEvents();
        m_window->poll_events();
    }

    void WindowingModule::swap_buffers(f64 max_fps) const
    {
        assert(m_window != nullptr);
        m_window->swap_buffers(max_fps);
    }

    IWindow& WindowingModule::window() const
    {
        assert(m_window != nullptr);
        return *m_window;
    }
}