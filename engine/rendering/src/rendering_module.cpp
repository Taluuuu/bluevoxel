#include "rendering/rendering_module.h"

#include "core/engine.h"
#include "renderer_vulkan.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

#include <iostream>
#include <typeinfo>

namespace engine
{
    RenderingModule::RenderingModule(Engine& engine)
        : Module(engine) {}

    bool RenderingModule::init(const AppInfo& app_info)
    {
        Module::init(app_info);

        const auto windowing_module = engine().get_module<WindowingModule>();
        assert(windowing_module != nullptr);
        const auto& window = windowing_module->window();

        try
        {
            m_renderer = std::make_shared<Renderer_Vulkan>(app_info, window);
        }
        catch(const std::exception& e)
        {
            std::cout << "[Error] " << e.what() << "\n";
            return false;
        }

        return true;
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