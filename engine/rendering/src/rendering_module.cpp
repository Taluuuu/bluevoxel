#include "rendering/rendering_module.h"

#include "core/engine.h"
#include "vulkan/renderer_vulkan.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

#include <iostream>
#include <typeinfo>

namespace engine
{
    RenderingModule::RenderingModule(Engine& engine)
        : Module(engine) {}

    void RenderingModule::tick(f64 delta_time)
    {
        assert(m_renderer != nullptr);
        m_renderer->draw_frame();
    }

    bool RenderingModule::init(const GameInfo &game_info)
    {
        Module::init(game_info);

        const auto windowing_module = engine().get_module<WindowingModule>();
        assert(windowing_module != nullptr);
        const auto& window = windowing_module->window();

        try
        {
            m_renderer = std::make_shared<Renderer_Vulkan>(game_info, window);
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }

    void RenderingModule::cleanup()
    {
        m_renderer.reset();
    }

    std::vector<std::type_index> RenderingModule::get_dependencies() const
    {
        return { typeid(WindowingModule) };
    }

    IRenderer& RenderingModule::renderer() const
    {
        assert(m_renderer != nullptr);
        return *m_renderer;
    }
}