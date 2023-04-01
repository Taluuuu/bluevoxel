#include "rendering/scene/camera_component.h"

#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/scene/rendering_scene_system.h"
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
    }

    void CameraComponent::set_as_main_camera() const
    {
        auto render_system = m_scene->get_system<RenderingSceneSystem>();
        if (render_system)
            render_system->set_main_camera(m_camera);
    }
}