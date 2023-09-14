#pragma once

#include "core/module.h"
#include "networking/client.h"

namespace bluevoxel
{
    class BlueVoxelClientModule
        : public h2o::Tickable
        , public h2o::IModule
    {
    public:

        ~BlueVoxelClientModule() override = default;

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        // h2o::IModule interface
        [[nodiscard]] bool init(h2o::Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "bluevoxel_client_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        std::string m_server_ip { "127.0.0.1" };
        i32 m_server_port { 1338 };

        h2o::Client m_client{};

    };
}