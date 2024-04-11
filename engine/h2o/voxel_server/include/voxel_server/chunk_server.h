#pragma once

#include "core/events.h"
#include "core/types.h"
#include "networking/networking_types.h"
#include "chunk_manager_server.h"
#include "scene/scene_system.h"
#include "voxel/chunk_container_interface.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"
#include "voxel/chunk_generators/chunk_generator_base.h"
#include "voxel/voxel_net_messages.h"
#include "world_generator.h"

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace h2o
{
    class ChunkGenerator_Base;
    class INetPeer;

    class ChunkServer : public SceneSystem
    {
    public:

        explicit ChunkServer(
            const SceneSystemInitializer& system_initializer,
            INetPeer& server);
        ~ChunkServer() override = default;

        [[nodiscard]] IChunkManager& chunk_mgr() { return m_chunk_mgr; }
        [[nodiscard]] WorldGenerator& world_generator() { return m_world_generator; }

        void update(f32 delta_time) override;

    protected:

        // Networking
        void on_received_chunk_fetch_requests(
            PeerID client_id,
            const net_msg::ChunkFetchRequest& chunk_fetch_request);

        void on_received_block_place_request(
            PeerID request_sender,
            const net_msg::BlockPlaceRequest& block_place_request);

        void send_chunk_column(const ChunkColumn& chunk_col, const std::set<PeerID>& client_ids) const;

    private:

        // Networking
        INetPeer* const m_server = nullptr;
        EventHandle m_received_chunk_request_handle{};
        EventHandle m_received_block_place_request_handle{};

        // Storage
        ChunkManager_Server m_chunk_mgr{};

        // Generation
        WorldGenerator m_world_generator;

    };
}