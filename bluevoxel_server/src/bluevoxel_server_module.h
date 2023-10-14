#pragma once

#include "core/events.h"
#include "core/module.h"
#include "networking/server.h"

#include <memory>

namespace h2o
{
    class ChunkServer;
    class Scene;
}

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
        std::unique_ptr<h2o::ChunkServer> m_chunk_server { nullptr };

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        h2o::EventHandle m_player_joined_event_handle{};
        h2o::EventHandle m_on_received_transform_update_handle{};

    };
}