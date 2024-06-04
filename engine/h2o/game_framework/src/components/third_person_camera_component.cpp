#include "game_framework/components/third_person_camera_component.h"

#include "input/input_component.h"
#include "scene/actor.h"

namespace h2o
{
    ThirdPersonCameraComponent::ThirdPersonCameraComponent(const ComponentInitializer& component_initializer)
        : CameraComponent(component_initializer)
    {
        m_input = owner()->get_component<InputComponent>();

        if (!m_input)
            log::warn("Created FPS camera component without input component.");

        set_tick_phases(TickPhase::PreRender | TickPhase::Update);
    }

    void ThirdPersonCameraComponent::update(f32 delta_time)
    {
        if (!m_input)
            return;

        // Don't rotate camera if a higher priority mouse capture is active
        bool can_control = false;
        if (const auto mouse_capture_priority = m_input->mouse_capture_priority())
            can_control = *mouse_capture_priority <= MouseCapturePriority::Camera;

        if (can_control)
        {
            // Camera scroll
            distance_with_actor -= distance_with_actor * scroll_zoom_factor * m_input->get_axis("cam_zoom");
            distance_with_actor = glm::clamp(distance_with_actor, min_distance_with_actor, max_distance_with_actor);
        }

        bool wants_rotate = true;
        if (rotation_mouse_button)
            wants_rotate = m_input->mouse_button_state(*rotation_mouse_button).held;

        m_input->set_capture_mouse(MouseCapturePriority::Camera, wants_rotate);

        if (wants_rotate)
        {
            if (can_control)
            {
                // Camera rotation
                const v3 cam_input{
                    m_input->get_axis("cam_x"),
                    m_input->get_axis("cam_y"), 0.0f};

                owner()->transform.rotation += cam_input;
                owner()->transform.rotation.x = glm::clamp(owner()->transform.rotation.x,
                    -glm::half_pi<f32>() + 0.01f, glm::half_pi<f32>() - 0.01f);
            }
        }
    }

    v3 ThirdPersonCameraComponent::camera_location() const
    {
        const Transform& transform = owner()->transform;
        const f32 pitch = transform.rotation.x;
        const f32 yaw   = transform.rotation.y;

        // TODO: Add this function to transform class or somewhere it can be accessed
        //       easily, such as in the camera class.
        const v3 front = glm::normalize(v3(
            glm::cos(yaw) * glm::cos(pitch),
            glm::sin(pitch),
            glm::sin(yaw) * glm::cos(pitch)
        ));

        return transform.position - front * distance_with_actor;
    }
}