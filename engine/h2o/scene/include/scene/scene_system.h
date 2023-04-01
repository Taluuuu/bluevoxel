#pragma once

namespace h2o
{
    class Scene;

    struct SceneSystemInitializer
    {
        Scene& owning_scene;
    };

    class SceneSystem
    {
    public:

        explicit SceneSystem(const SceneSystemInitializer& system_initializer);
        virtual ~SceneSystem() = default;

    protected:

        // The lifetime of a system is strictly inferior to its
        // owning scene, so a raw pointer here is fine
        Scene* m_scene = nullptr;

    };
}