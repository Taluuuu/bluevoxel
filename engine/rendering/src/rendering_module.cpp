#include "rendering/rendering_module.h"

#include "core/engine.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

#if TNT_USE_VULKAN
#include "vulkan/renderer_vulkan.h"
#endif

#if TNT_USE_OPENGL
#include "opengl/renderer_opengl.h"
#endif

namespace h2o
{
    RenderingModule::RenderingModule(Engine& engine)
        : Module(engine) {}

    void RenderingModule::tick(f64 delta_time)
    {
        assert(m_renderer != nullptr);
        m_renderer->clear();
    }

    bool RenderingModule::init(const GameInfo& game_info)
    {
        Module::init(game_info);

        const auto windowing_module = m_engine->get_module<WindowingModule>();
        assert(windowing_module != nullptr);
        auto& window = windowing_module->window();

#if TNT_USE_VULKAN
        m_renderer = std::make_unique<Renderer_Vulkan>();
#elif TNT_USE_OPENGL
        m_renderer = std::make_unique<gfx::Renderer_OpenGL>();
#endif

        return m_renderer->init(window, game_info);
    }

    void RenderingModule::cleanup()
    {
        m_renderer.reset();
    }

    std::vector<std::type_index> RenderingModule::dependencies() const
    {
        return { typeid(WindowingModule) };
    }

    gfx::IRenderer& RenderingModule::renderer() const
    {
        assert(m_renderer != nullptr);
        return *m_renderer;
    }
}