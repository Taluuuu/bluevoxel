#include "game_framework/actors/fps_character_actor.h"

#include "core/engine.h"
#include "game_framework/components/fps_camera_component.h"
#include "input/input_component.h"
#include "physics/scene/collider_component.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        m_input = add_component<InputComponent>();

        m_collider = add_component<ColliderComponent>();
        m_collider->set_size(v3{ 1.0f });
        m_collider->set_offset(v3{ -0.5f });

        add_component<FpsCameraComponent>();

        set_tick_phases(TickPhase::Update);
    }

    void FpsCharacterActor::update(f32 delta_time)
    {
        // if (g_engine->layer_stack().top_layer() != Layer::Game)
        //     return;

        assert(m_collider);

        v2 move_input {
            m_input->get_axis("move_y"),
            m_input->get_axis("move_x") };

        if (glm::length2(move_input) > 0.1f)
            move_input = glm::normalize(move_input);

        const f32 rot_y = transform.rotation.y;
        const v3 move_input_rotated {
            move_input.x * glm::cos(rot_y) - move_input.y * glm::sin(rot_y),
            m_input->get_axis("fly"),
            move_input.x * glm::sin(rot_y) + move_input.y * glm::cos(rot_y) };

        transform.position += move_input_rotated * move_speed * delta_time;
    }
}