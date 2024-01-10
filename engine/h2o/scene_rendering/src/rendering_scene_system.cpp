#include "scene_rendering/rendering_scene_system.h"

#include "core/log.h"
#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/texture.h"
#include "scene/actor.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "windowing/windowing_module.h"
#include "windowing/window.h"

namespace h2o
{
    RenderingSystem::RenderingSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        auto rendering_module = g_engine->get_module<RenderingModule>();
        auto windowing_module = g_engine->get_module<WindowingModule>();
        assert(rendering_module && windowing_module);

        // Window resize logic
        v2i fb_size = windowing_module->window().framebuffer_size();
        m_aspect_ratio = (f32)fb_size.x / (f32)fb_size.y;

        windowing_module->window().resize_event().add_listener(m_resize_event_handle,
            [&](const WindowResizeEvent& event)
            {
                m_aspect_ratio = (f32)event.new_size.x / (f32)event.new_size.y;
                if (m_main_camera)
                    m_main_camera->aspect_ratio = m_aspect_ratio;
            });

        // Pipeline setup
        m_renderer = &rendering_module->renderer();

        // TODO: Move this to the system's init function
        m_pipeline = (*m_renderer)
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex,   "../Resources/engine/shaders/triangle.vert")
            .add_shader(gfx::ShaderStage::Fragment, "../Resources/engine/shaders/triangle.frag")
            .with_feature(gfx::PipelineFeature::CullFace)
            .with_feature(gfx::PipelineFeature::DepthTest)
            .compile();

        set_tick_phases(TickPhase::Render);
    }

    RenderingSystem::~RenderingSystem()
    {
        assert(m_mesh_renderer_components.empty());
    }

    void RenderingSystem::render()
    {
        if (!m_main_camera)
        {
            log::warn("No main camera is attached to the Rendering Scene System.");
            return;
        }

        if (!m_pipeline)
        {
            log::warn("No pipeline bound on draw for Rendering Scene System.");
            return;
        }

        m4 proj_view = m_main_camera->calc_proj_view();
        m_pipeline->set_uniform_mat4(0, proj_view);

        m_renderer->bind_pipeline(m_pipeline);
        for (const auto& render_comp : m_mesh_renderer_components)
        {
            // TODO: Setting a weak handle not as a reference
            //       for temporary use is inefficient

            auto actor = render_comp->owner();
            m_pipeline->set_uniform_mat4(1, actor->transform.model_matrix());
            m_pipeline->set_uniform_int(2, 0);

            if (const auto& mesh = render_comp->mesh())
            {
                if (const auto& texture = render_comp->texture())
                    texture->bind(0);

                m_renderer->draw(*mesh);
            }
        }
    }

    void RenderingSystem::register_component(const MeshRendererComponent& renderer_component)
    {
#ifndef NDEBUG
        auto result = std::find(
            m_mesh_renderer_components.begin(),
            m_mesh_renderer_components.end(),
            &renderer_component);

        assert(result == m_mesh_renderer_components.end());
#endif

        m_mesh_renderer_components.push_back(&renderer_component);
    }

    void RenderingSystem::unregister_component(const MeshRendererComponent& renderer_component)
    {
        [[maybe_unused]] auto num_erased = std::erase(m_mesh_renderer_components, &renderer_component);
        assert(num_erased == 1);
    }

    void RenderingSystem::set_main_camera(const WeakHandle<gfx::Camera>& camera)
    {
        m_main_camera = camera;
        if (m_main_camera)
            m_main_camera->aspect_ratio = m_aspect_ratio;
    }
}