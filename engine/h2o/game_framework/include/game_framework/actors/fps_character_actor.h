#pragma once

#include "core/events.h"
#include "scene/actor.h"

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

        f32 acceleration = 40.0f;       // m/s^2
        f32 deceleration = 75.0f;       // m/s^2
        f32 deceleration_angle = 45.0f; // degrees
        f32 max_speed = 5.0f;           // m/s

        bool fly = false;
        f32 gravity = 30.0f;            // m/s^2
        f32 jump_speed = 8.5f;          // m/s
        f32 fly_speed = 50.0f;          // m/s

    private:

        [[nodiscard]] v3 get_desired_move_dir() const;

        void update_fly(f32 delta_time);
        void update_walk(f32 delta_time);
        void update_fall(f32 delta_time);

    private:

        v3 m_velocity{};

        WeakHandle<InputComponent> m_input = nullptr;
        WeakHandle<ColliderComponent> m_collider = nullptr;

        EventHandle m_on_collision_handle{};

    };
}
