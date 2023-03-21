#pragma once

#include "core/module.h"
#include "core/core_interfaces.h"

#include <vector>

namespace h2o
{
    class Scene;

    class SceneModule
        : public IModule
        , public ITickable
    {
    public:

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_scene"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override { return {}; }

        // ITickable interface
        void tick(TickPhase phase, f64 delta_time) override;

        void register_scene(Scene& scene);
        void unregister_scene(Scene& scene);

    private:

        std::vector<Scene*> m_scenes;

    };
}