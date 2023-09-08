#pragma once

#include "core/module.h"

namespace h2o
{
    class VoxelClientModule : public IModule
    {
    public:

        ~VoxelClientModule() override = default;

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override;
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}