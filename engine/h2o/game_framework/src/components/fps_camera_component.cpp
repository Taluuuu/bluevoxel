#include "game_framework/components/fps_camera_component.h"

#include "input/input_component.h"
#include "scene/actor.h"

namespace h2o
{
    FpsCameraComponent::FpsCameraComponent(const ComponentInitializer& component_initializer)
        : CameraComponent(component_initializer)
    {
        m_input = owner()->get_component<InputComponent>();

        // TODO: Create start function called when all components are added.
        //       Also fix in ThirdPersonCameraComponent.
        if (!m_input)
            log::warn("Created FPS camera component without input component.");

        // TODO: Allow tick phases to OR with parent class
        //       Also fix in ThirdPersonCameraComponent.
        set_tick_phases(TickPhase::PreRender | TickPhase::Update);
    }

    void FpsCameraComponent::update(f32 delta_time)
    {
        if (!m_input)
            return;

        const v3 cam_input {
            m_input->get_axis("cam_x"),
            m_input->get_axis("cam_y"), 0.0f };

        owner()->transform.rotation += cam_input * mouse_sensitivity;
        owner()->transform.rotation.x = glm::clamp(owner()->transform.rotation.x, -89.0f, 89.0f);
    }
}