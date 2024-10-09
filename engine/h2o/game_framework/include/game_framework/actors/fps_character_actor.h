#pragma once

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

        f32 move_speed = 10.0f;

    private:

        WeakHandle<InputComponent> m_input = nullptr;
        WeakHandle<ColliderComponent> m_collider = nullptr;

    };
}