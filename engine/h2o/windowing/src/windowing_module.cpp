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
        m_window.reset();
        glfwTerminate();
    }

    WindowingModule::WindowingModule()
        : Tickable(g_engine)
    {}

    bool WindowingModule::init(Engine& engine)
    {
        if (!glfwInit())
        {
            log::error("Failed to initialize GLFW.");
            return false;
        }

        m_window.reset(Window_GLFW::create(engine.game_info().game_name, default_size));
        if (!m_window)
        {
            glfwTerminate();
            return false;
        }

        set_tick_phases(TickPhase::FrameStart | TickPhase::FrameEnd);

        return true;
    }

    f64 WindowingModule::delta_time() const
    {
        assert(m_window != nullptr);
        return m_window->delta_time();
    }

    f64 WindowingModule::time() const
    {
        assert(m_window != nullptr);
        return m_window->time();
    }

    IWindow& WindowingModule::window() const
    {
        assert(m_window != nullptr);
        return *m_window;
    }

    void WindowingModule::frame_start()
    {
        assert(m_window != nullptr);
        glfwPollEvents();

        g_engine->should_close = m_window->should_close();
    }

    void WindowingModule::frame_end(const f32 delta_time)
    {
        assert(m_window != nullptr);
        m_window->swap_buffers();
    }
}
