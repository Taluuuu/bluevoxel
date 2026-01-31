#include "game_framework/systems/player_movement_system.h"

#include "core/engine.h"
#include "game_framework/components/player_movement_component.h"
#include "glm/gtx/norm.hpp"
#include "scene/scene.h"
#include "scene/scene_networking_components.h"
#include "scene/transform.h"

namespace h2o
{
    PlayerMovementSystem::PlayerMovementSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
        , m_input_module(g_engine->get_module_checked<InputModule>())
    {
        set_tick_phases(TickPhase::Update);
    }

    void PlayerMovementSystem::update(f32 delta_time)
    {
        // Limit movement in a lag spike.
        // TODO: Investigate some better solution for this
        delta_time = glm::clamp(delta_time, 0.0f, 0.1f);

        auto& registry = scene.registry();
        const auto view = registry.view<PlayerMovementComp, Position, Rotation, Velocity>();
        view.each(
            [&](const entt::entity entity, PlayerMovementComp& movement_comp, Position& position, const Rotation& rotation, Velocity& velocity)
            {
                // Toggle fly - maybe move into another system ?
                if (m_input_module.key_state(Key::H).pressed_this_frame)
                    movement_comp.fly = !movement_comp.fly;

                // Toggle collider
                // if (m_input_module.key_state(Key::J).pressed_this_frame)
                // {
                //     if (m_collider)
                //         m_collider->set_enabled(!m_collider->is_enabled());
                // }

                const bool is_sprinting = m_input_module.key_state(Key::LeftShift).held;
                // if (m_camera)
                // {
                //     m_camera->target_fov_modifier =
                //         (m_is_sprinting && glm::length(m_velocity) > 1.0f) ? sprint_fov_modifier : 1.0f;
                // }

                if (movement_comp.fly)
                {
                    update_fly(delta_time, movement_comp, is_sprinting, rotation.rotation.y, velocity.velocity);
                }
                else
                {
                    if (touching_grass())
                    {
                        update_walk(delta_time, movement_comp, is_sprinting, rotation.rotation.y, velocity.velocity);

                        // Jump
                        const f32 current_time = g_engine->time_provider().time();
                        if (m_input_module.key_state(Key::Space).held && current_time > movement_comp.last_jump_time + movement_comp.min_time_between_jumps)
                        {
                            movement_comp.last_jump_time = current_time;
                            velocity.velocity.y = movement_comp.jump_speed;
                        }
                    }
                    else
                    {
                        update_fall(delta_time, movement_comp, is_sprinting, rotation.rotation.y, velocity.velocity);
                    }

                    // Apply gravity
                    velocity.velocity -= v3{ 0.0f, movement_comp.gravity * delta_time, 0.0f };
                }

                // TODO: Move this and gravity into a physics system
                position.position += velocity.velocity * delta_time;

                if (!registry.any_of<Dirty<Velocity>>(entity))
                    registry.emplace<Dirty<Velocity>>(entity);

                if (!registry.any_of<Dirty<Position>>(entity))
                    registry.emplace<Dirty<Position>>(entity);
            }
        );
    }

    v3 PlayerMovementSystem::get_desired_move_dir(const f32 rot_y) const
    {
        v3 move_input {
            -m_input_module.get_axis("move_x"), 0.0f,
            m_input_module.get_axis("move_y") };

        if (glm::length2(move_input) < 0.1f)
            return v3{ 0.0f };

        move_input = glm::normalize(move_input);

        const m4 rotation_matrix = glm::rotate(m4(1.0f), rot_y, v3{ 0.0f, 1.0f, 0.0f });
        const v3 move_dir_world = v3(rotation_matrix * v4(move_input, 0.0f));

        return move_dir_world;
    }

    bool PlayerMovementSystem::touching_grass() const
    {
        // // Test environment collision at the player's feet
        // if (m_collider && m_physics_system)
        // {
        //     // TODO: Fix this to make the feet collider smaller on the X and Z axes
        //     //       to prevent climbing walls just by holding space next to them
        //     // const physics::Collider_AABB feet_collider {
        //     //     .position = owner.transform.position + v3{ 0.0f, -0.1f, 0.0f },
        //     //     .size =
        //     // };
        //     auto feet_collider = m_collider->calc_collider();
        //     feet_collider.size.y = 0.1f;
        //     feet_collider.position.y -= 0.05f;
        //
        //     return m_physics_system->collides(feet_collider);
        // }
        //
        // return false;

        return true;
    }

    v3 PlayerMovementSystem::steered_horizontal_movement(
        const v3& velocity,
        const f32 accel,
        const f32 decel,
        const f32 max_speed,
        const f32 rot_y,
        const f32 delta_time) const
    {
        const v3 move_dir = get_desired_move_dir(rot_y);
        const v3 current_horizontal_vel{ velocity.x, 0.0f, velocity.z };
        const f32 current_horizontal_speed = glm::length(current_horizontal_vel);

        const f32 target_speed = (glm::length(move_dir) < 0.001f) ? 0.0f : max_speed;
        const f32 mix_coeff = (target_speed < current_horizontal_speed) ? decel : accel;

        const v3 target_vel = move_dir * target_speed;
        const v3 result = glm::mix(current_horizontal_vel, target_vel, mix_coeff * delta_time);

        return v3{ result.x, velocity.y, result.z };
    }

    void PlayerMovementSystem::update_fly(
        const f32 delta_time,
        const PlayerMovementComp& movement_comp,
        const bool is_sprinting,
        const f32 rot_y,
        v3& velocity) const
    {
        velocity = steered_horizontal_movement(
            velocity,
            movement_comp.acceleration_fly,
            movement_comp.deceleration_fly,
            is_sprinting ? movement_comp.max_speed_fly_sprint : movement_comp.max_speed_fly,
            rot_y,
            delta_time);

        // Fly
        velocity.y = m_input_module.get_axis("fly") * movement_comp.fly_speed;
    }

    void PlayerMovementSystem::update_walk(
        const f32 delta_time,
        const PlayerMovementComp& movement_comp,
        const bool is_sprinting,
        const f32 rot_y,
        v3& velocity) const
    {
        velocity = steered_horizontal_movement(
            velocity,
            movement_comp.acceleration_walk,
            movement_comp.deceleration_walk,
            is_sprinting ? movement_comp.max_speed_walk_sprint : movement_comp.max_speed_walk,
            rot_y,
            delta_time);
    }

    void PlayerMovementSystem::update_fall(
        const f32 delta_time,
        const PlayerMovementComp& movement_comp,
        const bool is_sprinting,
        const f32 rot_y,
        v3& velocity) const
    {
        velocity = steered_horizontal_movement(
            velocity,
            movement_comp.acceleration_fall,
            movement_comp.deceleration_fall,
            is_sprinting ? movement_comp.max_speed_fall_sprint : movement_comp.max_speed_fall,
            rot_y,
            delta_time);
    }
}
