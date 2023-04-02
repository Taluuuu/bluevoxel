#include "game_framework/actors/fps_character_actor.h"

#include "scene_rendering/camera_component.h"
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
            m_input->get_axis("move_y"), 0.0f,
            m_input->get_axis("move_x") };

        transform.position += input * delta_time;
    }
}