#include "game_framework/actors/fps_character_actor.h"

#include "scene/components/camera_component.h"
#include "scene/components/transform_component.h"
#include "input/input_component.h"

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        add_component<TransformComponent>();
        add_component<CameraComponent>();
        m_input = add_component<InputComponent>();
    }

    void FpsCharacterActor::tick(f32 delta_time)
    {
        if (!m_input)
            return;

        const v2 input = {
            m_input->get_axis("move_x"),
            m_input->get_axis("move_y") };
    }
}