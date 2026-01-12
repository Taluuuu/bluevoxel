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

    class PlayerCharacter : public Actor
    {
    public:

        explicit PlayerCharacter(const ActorInitializer& actor_initializer);
        ~PlayerCharacter() override = default;

    private:

        WeakHandle<ColliderComponent> m_collider = nullptr;

    };
}
