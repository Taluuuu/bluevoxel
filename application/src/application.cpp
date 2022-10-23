#include "application.h"

#include "windowing/window.h"
#include "windowing/windowing_module.h"
#include "rendering/rendering_module.h"

#include <iostream>

void Application::run()
{
    try
    {
        if (!init())
        return;

        // Main loop
        while (!m_window->should_close())
            update();

        cleanup();  
    }
    catch (const std::exception& e)
    {
        std::cout << "[Error] Unexpected exception: '" << e.what() << "'\n";
    }     
}

bool Application::init()
{
    m_engine
        .add_module<engine::RenderingModule>()
        .add_module<engine::WindowingModule>();

    if (!m_engine.loaded_all_modules())
        std::cout << "[Warning] Not all modules were correctly initialized." << std::endl;

    // Create window
    if (auto windowing_module = m_engine.get_module<engine::WindowingModule>())
        m_window = windowing_module->create_window(800, 600, "haaa");
        
    if (!m_window)
        return false;

    return true;
}

void Application::update()
{
    m_window->poll_events();

    // Do stuff

    m_window->swap_buffers();
}

void Application::cleanup()
{
    m_engine.cleanup();
}