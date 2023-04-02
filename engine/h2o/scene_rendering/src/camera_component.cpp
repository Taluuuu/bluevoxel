#include "scene_rendering/camera_component.h"

#include "core/engine.h"
#include "rendering/camera.h"
#include "scene_rendering/rendering_scene_system.h"
#include "scene/scene.h"

namespace h2o
{
    CameraComponent::CameraComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {
        f32 aspect_ratio = 1.0f;

        const auto& transform = owner()->transform;
        m_camera = oup::make_observable_unique<gfx::Camera>(
            90.0f,
            aspect_ratio,
            transform.position,
            transform.rotation);

        set_as_main_camera();

        g_engine->register_tickable(this, TickPhase::PostUpdate);
    }

    void CameraComponent::tick(h2o::TickPhase phase, f64 delta_time)
    {
        // TODO: Transform ref in components
        assert(owner());
        auto& transform = owner()->transform;

        if (m_camera)
            m_camera->update(transform.position, transform.rotation);
    }

    void CameraComponent::set_as_main_camera() const
    {
        auto render_system = m_scene->get_system<RenderingSceneSystem>();
        if (render_system)
            render_system->set_main_camera(m_camera);
    }
}