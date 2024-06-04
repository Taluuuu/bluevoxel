#pragma once

#include "core/module.h"
#include "core/tickable.h"

#include <set>

namespace h2o
{
    class Scene;

    class SceneModule : public IModule
    {
    public:

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_scene"; }

        void register_scene(Scene& scene);
        void unregister_scene(Scene& scene);

        void set_current_scene(Scene* scene);

    private:

        std::set<Scene*> m_scenes{};

        Scene* m_current_scene = nullptr;

    };
}