#pragma once

#include "core/module.h"

namespace bluevoxel
{
    class BlueVoxelServerModule : public h2o::IModule
    {
    public:

        ~BlueVoxelServerModule() override = default;

        // h2o::IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "bluevoxel_server_module"; }
        [[nodiscard]] virtual std::vector<std::type_index> dependencies() const override;

    };
}