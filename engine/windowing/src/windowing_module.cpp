#include "windowing/windowing_module.h"

#include "core/game_info.h"
#include "window_glfw.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace h2o
{
    WindowingModule::WindowingModule(Engine& engine)
        : Module(engine) {}

    bool WindowingModule::init(const GameInfo& game_info)
    {
        Module::init(game_info);

        if (!glfwInit())
            return false;

        try
        {
            m_window = Window_GLFW::create(game_info.game_name, default_size);
        }
        catch(const std::exception& e)
        {
            std::cout << "[Error] " << e.what() << "\n";
            glfwTerminate();
            return false;
        }

        return true;
    }

    void WindowingModule::cleanup()
    {
        m_window.reset();
        glfwTerminate();

        Module::cleanup();
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