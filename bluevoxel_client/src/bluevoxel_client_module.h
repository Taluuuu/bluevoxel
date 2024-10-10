#pragma once

#include "core/events.h"
#include "core/handle_types.h"
#include "core/module.h"
#include "networking/client.h"
#include "scene/actor_initializer.h"
#include "voxel/block.h"

#include <memory>

namespace h2o
{
    class ChunkClient;
    class InputModule;
    class Scene;
    class Scene_ECS;
    class Transform;
    class UIModule;
}

namespace bluevoxel
{
    class BlueVoxelClientModule
        : public h2o::Tickable
        , public h2o::IModule
    {
    public:

        BlueVoxelClientModule();
        ~BlueVoxelClientModule() override = default;

        // h2o::IModule interface
        [[nodiscard]] bool init(h2o::Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "bluevoxel_client"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // h2o::Tickable interface
        void update(f32 delta_time) override;

    private:

        void create_scene();
        void spawn_local_player();
        void spawn_remote_player(h2o::ActorID actor_id, const h2o::Transform& spawn_transform);

    private:

        std::string m_server_ip { "127.0.0.1" };
        i32 m_server_port = 1338;

        h2o::Client m_client;

        std::shared_ptr<h2o::Scene_ECS> m_scene = nullptr;
        // std::shared_ptr<h2o::Scene> m_scene = nullptr;
        h2o::WeakHandle<h2o::ChunkClient> m_chunk_client = nullptr;

        h2o::EventHandle m_on_client_created_handle{};
        h2o::EventHandle m_on_client_connected_to_server_handle{};
        h2o::EventHandle m_on_connected_handle{};
        h2o::EventHandle m_on_disconnected_handle{};

        // TEMP
        h2o::EventHandle m_on_testing_collisions_handle{};

        h2o::InputModule* m_input_module = nullptr;

    };
}