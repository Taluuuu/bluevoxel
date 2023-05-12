#pragma once

#include "core/tickable.h"

namespace h2o
{
    class Scene;

    struct SceneSystemInitializer
    {
        Scene& owning_scene;
    };

    class SceneSystem : public Tickable
    {
    public:

        explicit SceneSystem(const SceneSystemInitializer& system_initializer);
        ~SceneSystem() override = default;

        virtual bool init() { return true; }
        virtual void begin_play() {}
        virtual void end_play() {}

    protected:

        // The lifetime of a system is strictly inferior to its
        // owning scene, so a raw pointer here is fine
        Scene* const m_scene = nullptr;

    };
}