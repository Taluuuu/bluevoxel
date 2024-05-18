#include "rendering/rendering_module.h"

#include "core/engine.h"
#include "opengl/renderer_opengl.h"
#include "rendering/pipeline.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

namespace h2o
{
    RenderingModule::RenderingModule()
        : Tickable(g_engine)
    {}

    bool RenderingModule::init(Engine& engine)
    {
        set_tick_phases(TickPhase::FrameStart | TickPhase::PostRender | TickPhase::FrameEnd);

        const auto windowing_module = engine.get_module<WindowingModule>();
        assert(windowing_module != nullptr);
        auto& window = windowing_module->window();

        m_renderer = std::make_shared<gfx::Renderer_OpenGL>();

        return m_renderer->init(window, engine.game_info());
    }

    void RenderingModule::cleanup()
    {
        m_renderer->cleanup();
    }

    std::vector<std::type_index> RenderingModule::dependencies() const
    {
        return { typeid(WindowingModule) };
    }

    void RenderingModule::frame_start(f32 delta_time)
    {
        m_renderer->start_frame();
    }

    void RenderingModule::post_render()
    {
        m_renderer->post_render();
    }

    void RenderingModule::frame_end(f32 delta_time)
    {
        m_renderer->end_frame();
    }

    gfx::IRenderer& RenderingModule::renderer() const
    {
        assert(m_renderer);
        return *m_renderer;
    }

    gfx::Renderer_Base& RenderingModule::renderer_base() const
    {
        assert(m_renderer);
        return *m_renderer;
    }
}