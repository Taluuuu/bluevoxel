#pragma once

#include "core/module.h"

namespace bluevoxel
{
    class ClientModule : public h2o::IModule
    {
    public:

        ~ClientModule() override = default;

        // h2o::IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "bluevoxel_client_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}