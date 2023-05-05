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
    bool RenderingModule::init(Engine& engine)
    {
        set_tick_phases(FrameStart | FrameEnd);

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

    void RenderingModule::frame_start(f32 delta_time)
    {
        assert(m_renderer);
        m_renderer->start_frame();
    }

    void RenderingModule::frame_end(f32 delta_time)
    {
        assert(m_renderer);
        m_renderer->end_frame();
    }

    gfx::IRenderer& RenderingModule::renderer() const
    {
        assert(m_renderer != nullptr);
        return *m_renderer;
    }
}