#include "scene_rendering/rendering_scene_system.h"

#include "core/log.h"
#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/texture.h"
#include "scene/actor.h"
#include "scene/scene.h"
#include "scene_rendering/camera_component.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "windowing/windowing_module.h"
#include "windowing/window.h"

#include <algorithm>

namespace h2o
{
    RenderingSystem::RenderingSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
        , m_renderer(g_engine->get_module_checked<RenderingModule>().renderer())
    {
        const auto& windowing_module = g_engine->get_module_checked<WindowingModule>();

        // Window resize logic
        v2i fb_size = windowing_module.window().framebuffer_size();
        m_aspect_ratio = (f32)fb_size.x / (f32)fb_size.y;

        windowing_module.window().resize_event().add_listener(m_resize_event_handle,
            [&](const WindowResizeEvent& event)
            {
                m_aspect_ratio = (f32)event.new_size.x / (f32)event.new_size.y;
                if (const auto camera = find_camera())
                    camera->camera.aspect_ratio = m_aspect_ratio;
            }
        );

        // TODO: Move this to the system's init function
        m_pipeline = m_renderer
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex,   "engine/shaders/triangle.vert")
            .add_shader(gfx::ShaderStage::Fragment, "engine/shaders/triangle.frag")
            .with_feature(gfx::PipelineFeature::CullFace)
            .with_feature(gfx::PipelineFeature::DepthTest)
            .compile();

        set_tick_phases(TickPhase::PreRender | TickPhase::Render);
    }

    RenderingSystem::~RenderingSystem()
    {
        assert(m_mesh_renderer_components.empty());
    }

    void RenderingSystem::pre_render()
    {
        const auto view = scene.registry().view<CameraComp, Position, Rotation>();
        const auto entity = view.front();
        if (entity == entt::null)
            return;

        auto& camera = view.get<CameraComp>(entity).camera;
        const auto& position = view.get<Position>(entity).position;
        const auto& rotation = view.get<Rotation>(entity).rotation;

        camera.aspect_ratio = m_aspect_ratio;
        camera.update(position, rotation);
        m_renderer.set_camera(camera);
    }

    void RenderingSystem::render()
    {
        const auto camera = find_camera();
        if (!camera)
            return;

        if (!m_pipeline)
        {
            log::warn("No pipeline bound on draw for Rendering Scene System.");
            return;
        }

        m_pipeline->set_uniform_mat4(0, m_renderer.proj_view_matrix());

        m_renderer.bind_pipeline(m_pipeline);
        for (const auto render_comp : m_mesh_renderer_components)
        {
            assert(render_comp);

            auto& actor = render_comp->owner;
            m_pipeline->set_uniform_mat4(1, actor.transform.model_matrix());
            m_pipeline->set_uniform_int(2, 0);

            if (const auto& mesh = render_comp->mesh())
            {
                if (const auto& texture = render_comp->texture())
                    texture->bind(0);

                m_renderer.draw(*mesh);
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

    CameraComp* RenderingSystem::find_camera() const
    {
        const auto view = scene.registry().view<CameraComp>();
        const auto entity = view.front();
        if (entity == entt::null)
            return nullptr;

        return &view.get<CameraComp>(entity);
    }
}
