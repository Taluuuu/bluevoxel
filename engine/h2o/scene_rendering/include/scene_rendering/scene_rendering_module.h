#pragma once

#include "core/module.h"

namespace h2o
{
    class SceneRenderingModule : public IModule
    {
    public:

        SceneRenderingModule() = default;
        ~SceneRenderingModule() override = default;

        [[nodiscard]] std::string_view module_name() const override { return "h2o_scene_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}