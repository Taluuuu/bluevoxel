#pragma once

#include "block_placeable_interface.h"
#include "core/distance_queue.h"
#include "scene/scene_system.h"
#include "voxel/chunk_manager.h"
#include "voxel/voxel_bounds.h"

#include <atomic>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <voxel_rendering/voxel_world_renderer.h>

namespace h2o
{
    class INetPeer;
    class RenderingModule;
    class VoxelModule;

    class ChunkClient
        : public SceneSystem
        , public IBlockPlaceable
    {
    public:

        explicit ChunkClient(
            const SceneSystemInitializer& system_initializer,
            INetPeer& client);
        ~ChunkClient() override = default;

        // IBlockPlaceable interface
        void set_block_at(const v3i& block_pos, Block block) override;
        [[nodiscard]] ChunkManager& chunk_mgr() override { return m_chunk_mgr; }
        [[nodiscard]] const ChunkManager& chunk_mgr() const override { return m_chunk_mgr; }

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        void request_chunk_loads();

    private:

        ChunkManager m_chunk_mgr{};
        VoxelWorldRenderer m_voxel_world_renderer;

        bool m_refresh_chunk_requests = false;

        // Net event handles
        EventHandle m_on_connected_handle{};
        EventHandle m_on_fetched_chunk_handle{};
        EventHandle m_on_received_block_place_request_handle{};

        // Local events
        EventHandle m_on_testing_collisions_handle{};
        EventHandle m_on_raycasting_handle{};
        EventHandle m_on_block_placed_handle{};

        VoxelBounds m_voxel_bounds;
        v2i m_previous_player_chunk_col_pos{};
        v3 m_player_pos{};

        std::atomic_int m_num_chunk_columns_pending_decompress = 0;

        INetPeer*        m_client           = nullptr;
        RenderingModule* m_rendering_module = nullptr;
        VoxelModule*     m_voxel_module     = nullptr;

    };
}
