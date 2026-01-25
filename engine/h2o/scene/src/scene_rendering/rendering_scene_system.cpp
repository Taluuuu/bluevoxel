#include "scene_rendering/rendering_scene_system.h"

#include "core/log.h"
#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
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
    namespace
    {
        [[nodiscard]] m4 make_model_matrix(const v3& position, const v3& rotation, const v3& scale)
        {
            m4 model(1.0f);

            model = glm::translate(model, position);
            model = glm::rotate(model, rotation.y, { 0.0f, 1.0f, 0.0f });
            model = glm::rotate(model, rotation.x, { 1.0f, 0.0f, 0.0f });
            model = glm::rotate(model, rotation.z, { 0.0f, 0.0f, 1.0f });
            model = glm::scale(model, scale);

            return model;
        }
    }

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

        scene.registry().on_construct<MeshRenderer>().connect<&RenderingSystem::on_mesh_renderer_created>(*this);

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
        // assert(m_mesh_renderer_components.empty());
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

        scene.registry().view<MeshRenderer, Position, Rotation, Scale>().each(
            [&](
                const MeshRenderer& mesh_renderer,
                const Position& position,
                const Rotation& rotation,
                const Scale& scale)
            {
                m_pipeline->set_uniform_mat4(1, make_model_matrix(position.position, rotation.rotation, scale.scale));
                m_pipeline->set_uniform_int(2, 0);

                if (const auto& mesh = mesh_renderer.mesh)
                {
                    if (const auto& texture = mesh_renderer.texture)
                        texture->bind(0);

                    m_renderer.draw(*mesh);
                }
            }
        );
    }

    CameraComp* RenderingSystem::find_camera() const
    {
        const auto view = scene.registry().view<CameraComp>();
        const auto entity = view.front();
        if (entity == entt::null)
            return nullptr;

        return &view.get<CameraComp>(entity);
    }

    void RenderingSystem::on_mesh_renderer_created(const entt::entity entity) const
    {
        const auto mesh_renderer = scene.registry().try_get<MeshRenderer>(entity);
        if (!mesh_renderer)
            return;

        mesh_renderer->mesh = g_engine->resource_mgr().fetch<gfx::Mesh>(mesh_renderer->mesh_path);
        mesh_renderer->texture = g_engine->resource_mgr().fetch<gfx::Texture>(mesh_renderer->texture_path);
    }
}
