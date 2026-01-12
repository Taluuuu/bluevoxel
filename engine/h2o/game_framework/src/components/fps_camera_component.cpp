#include "game_framework/components/fps_camera_component.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "scene/actor.h"

namespace h2o
{
    FpsCameraComponent::FpsCameraComponent(const ComponentInitializer& component_initializer)
        : CameraComponent(component_initializer)
        , m_input_module(g_engine->get_module_checked<InputModule>())
    {
        // TODO: Allow tick phases to OR with parent class
        //       Also fix in ThirdPersonCameraComponent.
        set_tick_phases(TickPhase::PreRender | TickPhase::Update);
    }

    void FpsCameraComponent::update(const f32 delta_time)
    {
        CameraComponent::update(delta_time);

        g_engine->layer_stack().push_layer(Layer::Game, LayerData { true, false });

        const v3 cam_input {
            m_input_module.get_axis("cam_x"),
            m_input_module.get_axis("cam_y"), 0.0f };

        owner.transform.rotation += cam_input;
        owner.transform.rotation.x = glm::clamp(owner.transform.rotation.x,
            -glm::half_pi<f32>() + 0.01f, glm::half_pi<f32>() - 0.01f);
    }
}