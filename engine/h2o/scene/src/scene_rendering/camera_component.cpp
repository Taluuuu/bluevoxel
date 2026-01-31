#include "scene_rendering/camera_component.h"

#include "core/engine.h"
#include "rendering/camera.h"
#include "scene_rendering/rendering_scene_system.h"
#include "scene/scene.h"

namespace h2o
{
    // CameraComponent::CameraComponent(const ComponentInitializer& component_initializer)
    //     : Component(component_initializer)
    // {
    //     f32 aspect_ratio = 1.0f;
    //
    //     const auto& transform = owner.transform;
    //     m_camera = oup::make_observable_unique<gfx::Camera>(
    //         m_initial_fov,
    //         aspect_ratio,
    //         transform.position,
    //         transform.rotation);
    //
    //     set_as_main_camera();
    //
    //     set_tick_phases(TickPhase::Update | TickPhase::PreRender);
    // }
    //
    // void CameraComponent::update(const f32 delta_time)
    // {
    //     m_current_fov_modifier = glm::mix(
    //         m_current_fov_modifier, target_fov_modifier, fov_modifier_mix_coeff * delta_time);
    // }
    //
    // void CameraComponent::pre_render()
    // {
    //     if (m_camera)
    //     {
    //         m_camera->fov = m_initial_fov * m_current_fov_modifier;
    //         m_camera->update(camera_location(), camera_rotation());
    //     }
    // }
    //
    // void CameraComponent::set_as_main_camera() const
    // {
    //
    // }
    //
    // const gfx::Camera& CameraComponent::camera() const
    // {
    //     assert(m_camera);
    //     return *m_camera;
    // }
    //
    // v3 CameraComponent::camera_location() const
    // {
    //     return owner.transform.position;
    // }
    //
    // v3 CameraComponent::camera_rotation() const
    // {
    //     return owner.transform.rotation;
    // }
}