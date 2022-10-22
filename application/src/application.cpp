#include "application.h"

#include "windowing/window.h"
#include "windowing/windowing_module.h"
#include "rendering/rendering_module.h"

void Application::run()
{
    if (!init())
        return;

    // Main loop
    while (!m_window->should_close())
        update();

    cleanup();        
}

bool Application::init()
{
    engine::WindowingModule::init();
    engine::RenderingModule::init();

    m_window = engine::WindowingModule::create_window(800, 600, "haaa");
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
    engine::RenderingModule::cleanup();
    engine::WindowingModule::cleanup();
}