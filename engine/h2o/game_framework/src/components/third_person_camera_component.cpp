#include "game_framework/components/third_person_camera_component.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "scene/actor.h"

namespace h2o
{
    ThirdPersonCameraComponent::ThirdPersonCameraComponent(const ComponentInitializer& component_initializer)
        : CameraComponent(component_initializer)
        , m_input_module(g_engine->get_module_checked<InputModule>())
    {
        set_tick_phases(TickPhase::PreRender | TickPhase::Update);
    }

    void ThirdPersonCameraComponent::update(f32 delta_time)
    {
        auto& layer_stack = g_engine->layer_stack();

        // Don't rotate camera if a higher layer is active
        const bool can_control = layer_stack.top_layer() == Layer::Game;
        const bool wants_rotate = rotate_mouse_button && m_input_module.mouse_button_state(*rotate_mouse_button).held;

        const bool should_capture_mouse = can_control && wants_rotate;

        layer_stack.push_layer(Layer::Game,
            LayerData
            {
                .capture_mouse = should_capture_mouse,
                .allow_ui_interaction = !should_capture_mouse
            }
        );

        if (can_control)
        {
            // Camera scroll
            distance_with_actor -= distance_with_actor * scroll_zoom_factor * m_input_module.get_axis("cam_zoom");
            distance_with_actor = glm::clamp(distance_with_actor, min_distance_with_actor, max_distance_with_actor);

            if (wants_rotate)
            {
                // Camera rotation
                const v3 cam_input{
                    m_input_module.get_axis("cam_x"),
                    m_input_module.get_axis("cam_y"), 0.0f};

                owner.transform.rotation += cam_input;
                owner.transform.rotation.x = glm::clamp(owner.transform.rotation.x,
                    -glm::half_pi<f32>() + 0.01f, glm::half_pi<f32>() - 0.01f);
            }
        }
    }

    v3 ThirdPersonCameraComponent::camera_location() const
    {
        const Transform& transform = owner.transform;
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