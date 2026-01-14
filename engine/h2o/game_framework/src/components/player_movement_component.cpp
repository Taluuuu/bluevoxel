#include "game_framework/components/player_movement_component.h"

#include "core/engine.h"
#include "game_framework/components/fps_camera_component.h"
#include "input/input_module.h"
#include "physics/scene/physics_system.h"
#include "scene/actor.h"
#include "scene/scene.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    PlayerMovementComponent::PlayerMovementComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
        , m_input_module(g_engine->get_module_checked<InputModule>())
        , m_physics_system(scene.get_system<PhysicsSystem>()) // TODO: Should return a raw ptr eventually
    {
        set_tick_phases(TickPhase::Update);
    }

    void PlayerMovementComponent::start()
    {
        m_collider = owner.get_component<ColliderComponent>();
        if (m_collider)
        {
            m_collider->on_collision.add_listener(m_on_collision_handle,
                [this](const CollisionEvent& collision_event)
                {
                    // https://www.youtube.com/watch?v=oom6R-M2lvQ
                    const v3 normal = collision_event.normal;
                    m_velocity = m_velocity - normal * glm::dot(m_velocity, normal);
                }
            );
        }

        m_camera = owner.get_component<FpsCameraComponent>();
    }

    void PlayerMovementComponent::update(f32 delta_time)
    {
        // Limit movement in a lag spike
        delta_time = glm::clamp(delta_time, 0.0f, 0.1f);

        // Toggle fly
        if (m_input_module.key_state(Key::H).pressed_this_frame)
            fly = !fly;

        if (m_input_module.key_state(Key::J).pressed_this_frame)
        {
            if (m_collider)
                m_collider->set_enabled(!m_collider->is_enabled());
        }

        m_is_sprinting = m_input_module.key_state(Key::LeftShift).held;
        if (m_camera)
        {
            m_camera->target_fov_modifier =
                (m_is_sprinting && glm::length(m_velocity) > 1.0f) ? sprint_fov_modifier : 1.0f;
        }

        if (fly)
        {
            update_fly(delta_time);
        }
        else
        {
            if (touching_grass())
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

        owner.transform.position += m_velocity * delta_time;
    }

    v3 PlayerMovementComponent::get_desired_move_dir() const
    {
        v3 move_input {
            m_input_module.get_axis("move_y"), 0.0f,
            m_input_module.get_axis("move_x") };

        if (glm::length2(move_input) < 0.1f)
            return v3{ 0.0f };

        move_input = glm::normalize(move_input);

        const f32 rot_y = owner.transform.rotation.y;
        const v3 move_input_rotated {
            move_input.x * glm::cos(rot_y) - move_input.z * glm::sin(rot_y), 0.0f,
            move_input.x * glm::sin(rot_y) + move_input.z * glm::cos(rot_y) };

        return move_input_rotated;
    }

    bool PlayerMovementComponent::touching_grass() const
    {
        // Test environment collision at the player's feet
        if (m_collider && m_physics_system)
        {
            // TODO: Fix this to make the feet collider smaller on the X and Z axes
            //       to prevent climbing walls just by holding space next to them
            // const physics::Collider_AABB feet_collider {
            //     .position = owner.transform.position + v3{ 0.0f, -0.1f, 0.0f },
            //     .size =
            // };
            auto feet_collider = m_collider->calc_collider();
            feet_collider.size.y = 0.1f;
            feet_collider.position.y -= 0.05f;

            return m_physics_system->collides(feet_collider);
        }

        return false;
    }

    v3 PlayerMovementComponent::steered_horizontal_movement(
        const v3& velocity,
        const f32 accel,
        const f32 decel,
        const f32 max_speed,
        const f32 delta_time) const
    {
        const v3 move_dir = get_desired_move_dir();
        const v3 current_horizontal_vel{ velocity.x, 0.0f, velocity.z };
        const f32 current_horizontal_speed = glm::length(current_horizontal_vel);

        const f32 target_speed = (glm::length(move_dir) < 0.001f) ? 0.0f : max_speed;
        const f32 mix_coeff = (target_speed < current_horizontal_speed) ? decel : accel;

        const v3 target_vel = move_dir * target_speed;
        const v3 result = glm::mix(current_horizontal_vel, target_vel, mix_coeff * delta_time);

        return v3{ result.x, velocity.y, result.z };
    }

    void PlayerMovementComponent::update_fly(const f32 delta_time)
    {
        m_velocity = steered_horizontal_movement(
            m_velocity,
            acceleration_fly,
            deceleration_fly,
            m_is_sprinting ? max_speed_fly_sprint : max_speed_fly,
            delta_time);

        // Fly
        m_velocity.y = m_input_module.get_axis("fly") * fly_speed;
    }

    void PlayerMovementComponent::update_walk(const f32 delta_time)
    {
        m_velocity = steered_horizontal_movement(
            m_velocity,
            acceleration_walk,
            deceleration_walk,
            m_is_sprinting ? max_speed_walk_sprint : max_speed_walk,
            delta_time);

        // Jump
        const f32 current_time = g_engine->time_provider().time();
        if (m_input_module.key_state(Key::Space).held && current_time > m_last_jump_time + min_time_between_jumps)
        {
            m_last_jump_time = current_time;
            m_velocity.y = jump_speed;
        }
    }

    void PlayerMovementComponent::update_fall(const f32 delta_time)
    {
        m_velocity = steered_horizontal_movement(
            m_velocity,
            acceleration_fall,
            deceleration_fall,
            m_is_sprinting ? max_speed_fall_sprint : max_speed_fall,
            delta_time);
    }
}
