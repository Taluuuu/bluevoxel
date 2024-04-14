#pragma once

#include "core/module.h"

namespace h2o
{
    class InventoryModule : public IModule
    {
    public:

        InventoryModule() = default;
        ~InventoryModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_inventory"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}