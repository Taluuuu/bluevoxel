#pragma once

#include "physics/scene/physics_system.h"
#include "scene/component.h"

namespace h2o
{
    class FpsCameraComponent;
    class InputModule;

    class PlayerMovementComponent : public Component
    {
    public:

        explicit PlayerMovementComponent(const ComponentInitializer& component_initializer);

        void start() override;

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

        bool fly = true;
        f32 fly_speed = 20.0f; // m/s

        f32 sprint_fov_modifier = 1.1f;

    protected:

        void update(f32 delta_time) override;

    private:

        [[nodiscard]] v3 get_desired_move_dir() const;
        [[nodiscard]] bool touching_grass() const;
        [[nodiscard]] v3 steered_horizontal_movement(const v3& velocity, f32 accel, f32 decel, f32 max_speed, f32 delta_time) const;

        void update_fly(f32 delta_time);
        void update_walk(f32 delta_time);
        void update_fall(f32 delta_time);

    private:

        v3 m_velocity{};

        bool m_is_sprinting = false;

        f32 m_last_jump_time = 0.0f;

        ColliderComponent* m_collider = nullptr;
        FpsCameraComponent* m_camera = nullptr;

        InputModule& m_input_module;

        WeakHandle<PhysicsSystem> m_physics_system = nullptr;

        EventHandle m_on_collision_handle{};

    };

    struct PlayerMovementComp
    {
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

        bool fly = true;
        f32 fly_speed = 20.0f; // m/s

        f32 sprint_fov_modifier = 1.1f;

        f32 last_jump_time = 0.0f;
    };
}
