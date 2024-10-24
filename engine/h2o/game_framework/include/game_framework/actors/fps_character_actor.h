#pragma once

#include "core/events.h"
#include "scene/actor.h"

namespace h2o
{
    class FpsCameraComponent;
    class PhysicsSystem;
}

namespace h2o
{
    class InputComponent;
    class CameraComponent;
    class ColliderComponent;

    class FpsCharacterActor : public Actor
    {
    public:

        explicit FpsCharacterActor(const ActorInitializer& actor_initializer);
        ~FpsCharacterActor() override = default;

        void update(f32 delta_time) override;

    public:

        f32 acceleration_walk = 12.0f;
        f32 deceleration_walk = 20.0f;
        f32 max_speed_walk = 5.0f; // m/s
        f32 max_speed_walk_sprint = 7.0f; // m/s

        f32 acceleration_fall = 3.0f;
        f32 deceleration_fall = 1.0f;
        f32 max_speed_fall = 5.0f; // m/s
        f32 max_speed_fall_sprint = 8.0f; // m/s

        f32 acceleration_fly = 10.0f;
        f32 deceleration_fly = 20.0f;
        f32 max_speed_fly = 20.0f; // m/s
        f32 max_speed_fly_sprint = 30.0f; // m/s

        f32 gravity = 30.0f; // m/s^2

        f32 jump_speed = 8.5f; // m/s
        f32 min_time_between_jumps = 0.25f; // s

        bool fly = false;
        f32 fly_speed = 20.0f; // m/s

        f32 sprint_fov_modifier = 1.1f;

    private:

        [[nodiscard]] v3 get_desired_move_dir() const;
        [[nodiscard]] bool touching_grass() const;

        void update_fly(f32 delta_time);
        void update_walk(f32 delta_time);
        void update_fall(f32 delta_time);

        [[nodiscard]] v3 steered_horizontal_movement(const v3& velocity, f32 accel, f32 decel, f32 max_speed, f32 delta_time) const;

    private:

        v3 m_velocity{};

        bool m_is_sprinting = false;

        f32 m_last_jump_time = 0.0f;

        WeakHandle<InputComponent> m_input = nullptr;
        WeakHandle<ColliderComponent> m_collider = nullptr;
        WeakHandle<FpsCameraComponent> m_camera = nullptr;
        WeakHandle<PhysicsSystem> m_physics_system = nullptr;

        EventHandle m_on_collision_handle{};

    };
}
