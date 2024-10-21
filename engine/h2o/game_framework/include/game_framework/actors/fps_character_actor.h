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

        f32 acceleration = 30.0f; // m/s^2
        f32 deceleration = 30.0f; // m/s^2
        f32 max_speed = 10.0f;    // m/s

        f32 gravity = 9.8f;       // m/s^2
        f32 jump_speed = 10.0f;   // m/s

    private:

        v3 m_velocity{};

        WeakHandle<InputComponent> m_input = nullptr;
        WeakHandle<ColliderComponent> m_collider = nullptr;

        EventHandle m_on_collision_handle{};

    };
}
