#pragma once

#include "core/module.h"
#include "core/tickable.h"

#include <vector>

namespace h2o
{
    class Scene;

    class SceneModule : public IModule
    {
    public:

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_scene"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override { return {}; }

        void register_scene(Scene& scene);
        void unregister_scene(Scene& scene);

    private:

        std::vector<Scene*> m_scenes;

    };
}