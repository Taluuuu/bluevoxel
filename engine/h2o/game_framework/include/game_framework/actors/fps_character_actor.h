#pragma once

#include "scene/actor.h"

namespace h2o
{
    class InputComponent;
    class CameraComponent;

    class FpsCharacterActor : public Actor
    {
    public:

        explicit FpsCharacterActor(const ActorInitializer& actor_initializer);
        ~FpsCharacterActor() override = default;

        void update(f32 delta_time) override;

    public:

        f32 mouse_sensitivity = 1.0f;
        f32 move_speed = 10.0f;

    private:

        WeakHandle<InputComponent> m_input = nullptr;

    };
}