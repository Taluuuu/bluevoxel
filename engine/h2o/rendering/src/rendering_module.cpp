#include "rendering/rendering_module.h"

#include "core/engine.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

#if H2O_USE_VULKAN
#include "vulkan/renderer_vulkan.h"
#endif

#if H2O_USE_OPENGL
#include "opengl/renderer_opengl.h"
#endif

namespace h2o
{
    void RenderingModule::tick(TickPhase phase, f64 delta_time)
    {
        assert(m_renderer != nullptr);
        m_renderer->clear();
    }

    bool RenderingModule::init(Engine& engine)
    {
        engine.register_tickable(this, TickPhase::PreRender);

        const auto windowing_module = engine.get_module<WindowingModule>();
        assert(windowing_module != nullptr);
        auto& window = windowing_module->window();

#if H2O_USE_VULKAN
        m_renderer = std::make_unique<Renderer_Vulkan>();
#elif H2O_USE_OPENGL
        m_renderer = std::make_unique<gfx::Renderer_OpenGL>();
#endif

        return m_renderer->init(window, engine.game_info());
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