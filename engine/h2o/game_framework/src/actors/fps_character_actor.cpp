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
        m_collider->set_size(v3{ 0.8f, 1.7f, 0.8f });
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

        // Toggle fly
        if (m_input->key_state(Key::H).pressed_this_frame)
            fly = !fly;

        if (fly)
        {
            update_fly(delta_time);
        }
        else
        {
            // Bad but will work until i make a better raycast system to find the ground
            const bool touching_grass = m_velocity.y > -1.0f;

            if (touching_grass)
            {
                update_walk(delta_time);
            }
            else
            {
                update_fall(delta_time);
            }

            // Apply gravity
            m_velocity -= v3{ 0.0f, gravity * delta_time, 0.0f };
        }

        transform.position += m_velocity * delta_time;
    }

    v3 FpsCharacterActor::get_desired_move_dir() const
    {
        v3 move_input {
            m_input->get_axis("move_y"), 0.0f,
            m_input->get_axis("move_x") };

        if (glm::length2(move_input) < 0.1f)
            return v3{ 0.0f };

        move_input = glm::normalize(move_input);

        const f32 rot_y = transform.rotation.y;
        const v3 move_input_rotated {
            move_input.x * glm::cos(rot_y) - move_input.z * glm::sin(rot_y), 0.0f,
            move_input.x * glm::sin(rot_y) + move_input.z * glm::cos(rot_y) };

        return move_input_rotated;
    }

    void FpsCharacterActor::update_fly(const f32 delta_time)
    {
        const v3 move_dir = get_desired_move_dir();

        m_velocity = move_dir * fly_speed;
        m_velocity.y = m_input->get_axis("fly") * fly_speed;
    }

    void FpsCharacterActor::update_walk(const f32 delta_time)
    {
        const v3 move_dir = get_desired_move_dir();
        const v3 horizontal_vel{ m_velocity.x, 0.0f, m_velocity.z };

        if (move_dir != v3{ 0.0f })
        {
            // Accelerate...
            if (glm::length(horizontal_vel) > 0.1f &&
                glm::dot(move_dir, glm::normalize(horizontal_vel)) < glm::acos(glm::radians(deceleration_angle)))
            {
                // Use deceleration if going the opposite way enough
                m_velocity += deceleration * move_dir * delta_time;
            }
            else
            {
                m_velocity += acceleration * move_dir * delta_time;
            }
        }
        else
        {
            // Decelerate...
            const f32 horizontal_speed = glm::length(horizontal_vel);
            if (horizontal_speed > 0.0001f)
            {
                // Make sure we don't decelerate past 0
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

        // Cap horizontal speed to max_speed
        const f32 horizontal_speed = glm::length(v2{ m_velocity.x, m_velocity.z });
        if (horizontal_speed > max_speed)
        {
            const f32 temp = max_speed / horizontal_speed;
            m_velocity *= v3{ temp, 1.0f, temp };
        }

        // Jump
        if (m_input->key_state(Key::Space).pressed_this_frame)
            m_velocity.y = jump_speed;
    }

    void FpsCharacterActor::update_fall(const f32 delta_time)
    {
    }
}
