#include "game_framework/systems/fps_camera_system.h"

#include "core/engine.h"
#include "game_framework/components/fps_camera_component.h"
#include "input/input_module.h"
#include "scene/scene.h"
#include "scene/scene_networking_components.h"
#include "scene/transform.h"

namespace h2o
{
    FpsCameraSystem::FpsCameraSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
        , m_input_module(g_engine->get_module_checked<InputModule>())
    {
        set_tick_phases(TickPhase::Update);
    }

    void FpsCameraSystem::update(f32 delta_time)
    {
        g_engine->layer_stack().push_layer(Layer::Game, LayerData { true, false });

        const v3 cam_input {
            m_input_module.get_axis("cam_y"),
            m_input_module.get_axis("cam_x"), 0.0f };

        scene.registry().view<FpsCameraComp, Rotation>().each(
            [&](const entt::entity entity, Rotation& rotation)
            {
                rotation.rotation.y += cam_input.x;
                rotation.rotation.x += cam_input.y;

                rotation.rotation.x = glm::clamp(rotation.rotation.x,
                    -glm::half_pi<f32>() + 0.01f, glm::half_pi<f32>() - 0.01f);

                scene.registry().emplace_or_replace<Dirty<Rotation>>(entity);
            }
        );
    }
}
