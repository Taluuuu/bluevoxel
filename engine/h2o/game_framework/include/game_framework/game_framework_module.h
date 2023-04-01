#pragma once

#include "core/module.h"

namespace h2o
{
    class GameFrameworkModule : public IModule
    {
    public:

        ~GameFrameworkModule() override = default;

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_game_framework"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}