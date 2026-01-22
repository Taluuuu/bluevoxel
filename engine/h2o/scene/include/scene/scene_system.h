#pragma once

#include "core/tickable.h"

namespace h2o
{
    class Scene;

    struct SceneSystemInitializer
    {
        Scene& owning_scene;
        bool is_host = true;
    };

    class SceneSystem : public Tickable
    {
    public:

        explicit SceneSystem(const SceneSystemInitializer& system_initializer);
        ~SceneSystem() override = default;

    public:

        // The lifetime of a system is strictly inferior to its owning scene
        Scene& scene;

    };
}