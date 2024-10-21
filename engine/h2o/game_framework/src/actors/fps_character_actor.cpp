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
        m_collider->set_size(v3{ 0.8f, 1.8f, 0.8f });
        m_collider->set_offset(v3{ -0.4f, -1.5f, -0.4f });
        m_collider->on_collision.add_listener(m_on_collision_handle,
            [this](const CollisionEvent& collision_event)
            {
                // https://www.youtube.com/watch?v=oom6R-M2lvQ
                const v3 normal = collision_event.normal;
                m_velocity = m_velocity - normal * glm::dot(m_velocity, normal);
            }
        );

        add_component<FpsCameraComponent>();

        set_tick_phases(TickPhase::Update);
    }

    void FpsCharacterActor::update(const f32 delta_time)
    {
        // if (g_engine->layer_stack().top_layer() != Layer::Game)
        //     return;

        v2 move_input {
            m_input->get_axis("move_y"),
            m_input->get_axis("move_x") };

        if (glm::length2(move_input) > 0.1f)
        {
            // Accelerate...
            move_input = glm::normalize(move_input);

            const f32 rot_y = transform.rotation.y;
            const v3 move_input_rotated {
                move_input.x * glm::cos(rot_y) - move_input.y * glm::sin(rot_y), 0.0f,
                move_input.x * glm::sin(rot_y) + move_input.y * glm::cos(rot_y) };

            m_velocity += acceleration * move_input_rotated * delta_time;
        }
        else
        {
            // Decelerate...
            const v3 horizontal_vel{ m_velocity.x, 0.0f, m_velocity.z };
            const f32 horizontal_speed = glm::length(horizontal_vel);
            if (horizontal_speed > 0.0001f)
            {
                const f32 speed_change = deceleration * delta_time;
                v3 velocity_change;
                if (speed_change < horizontal_speed)
                {
                    const v3 horizontal_move_dir = glm::normalize(horizontal_vel);
                    velocity_change = -speed_change * horizontal_move_dir;
                }
                else
                {
                    velocity_change = -horizontal_vel;
                }

                m_velocity += velocity_change;
            }
        }

        m_velocity -= v3{ 0.0f, gravity * delta_time, 0.0f };

        if (m_input->key_state(Key::Space).pressed_this_frame)
            m_velocity.y = jump_speed;

        // Cap horizontal speed to max_speed
        const f32 horizontal_speed = glm::length(v2{ m_velocity.x, m_velocity.z });
        if (horizontal_speed > max_speed)
        {
            const f32 temp = max_speed / horizontal_speed;
            m_velocity *= v3{ temp, 1.0f, temp };
        }

        transform.position += m_velocity * delta_time;
    }
}