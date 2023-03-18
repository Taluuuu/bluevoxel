#pragma once

#include "core/module.h"

namespace h2o
{
    class SceneModule : public IModule
    {
    public:

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_scene"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override { return {}; }

    };
}