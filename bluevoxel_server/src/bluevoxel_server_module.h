#pragma once

#include "core/module.h"
#include "networking/server.h"

namespace bluevoxel
{
    class BlueVoxelServerModule : public h2o::IModule
    {
    public:

        ~BlueVoxelServerModule() override = default;

        // h2o::IModule interface
        [[nodiscard]] bool init(h2o::Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "bluevoxel_server_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        h2o::Server m_server{};

    };
}