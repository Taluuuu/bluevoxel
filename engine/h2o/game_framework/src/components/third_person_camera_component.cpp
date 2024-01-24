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

        // Camera scroll
        distance_with_actor -= distance_with_actor * scroll_zoom_factor * m_input->get_axis("cam_zoom");
        distance_with_actor = glm::clamp(distance_with_actor, min_distance_with_actor, max_distance_with_actor);

        const bool click_pressed = m_input->mouse_button_state(MouseButton::Left).held;
        m_input->set_capture_mouse(MouseCapturePriority::Camera, click_pressed);

        bool can_rotate = true;
        if (hold_click_to_rotate)
        {
            can_rotate = false;

            // Don't rotate camera if a higher priority mouse capture is active
            if (const auto mouse_capture_priority = m_input->mouse_capture_priority())
                can_rotate = *mouse_capture_priority <= MouseCapturePriority::Camera;
        }

        if (can_rotate)
        {
            // Camera rotation
            const v3 cam_input {
                m_input->get_axis("cam_x"),
                m_input->get_axis("cam_y"), 0.0f };

            owner()->transform.rotation += cam_input;
            owner()->transform.rotation.x = glm::clamp(owner()->transform.rotation.x, -89.0f, 89.0f);
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
            glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
        ));

        return transform.position - front * distance_with_actor;
    }
}