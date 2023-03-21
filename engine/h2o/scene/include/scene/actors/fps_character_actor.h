#pragma once

#include "scene/actor.h"

namespace h2o
{
    class CameraComponent;

    class FpsCharacterActor : public Actor
    {
    public:

        explicit FpsCharacterActor(const ActorInitializer& actor_initializer);
        ~FpsCharacterActor() override = default;

        void tick(f32 delta_time) override;

    private:

        WeakHandle<CameraComponent> m_camera_component = nullptr;

    };
}