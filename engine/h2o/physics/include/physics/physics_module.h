#pragma once

#include "core/module.h"

namespace h2o
{
    class PhysicsModule : public IModule
    {
    public:

        PhysicsModule() = default;
        ~PhysicsModule() override = default;

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_physics"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;
    };
}