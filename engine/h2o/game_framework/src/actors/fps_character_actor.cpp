#include "game_framework/actors/fps_character_actor.h"

#include "rendering/scene/camera_component.h"
#include "input/input_component.h"

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        m_camera = add_component<CameraComponent>();
        m_input = add_component<InputComponent>();
    }

    void FpsCharacterActor::tick(f32 delta_time)
    {
        assert(m_input);

        const v3 input = {
            m_input->get_axis("move_x"),
            0.0f,
            m_input->get_axis("move_y") };

        transform.position += v3(input.x, 0.0f, input.y) * delta_time;
    }
}