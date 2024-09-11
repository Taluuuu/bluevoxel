#pragma once

#include "voxel_rendering/chunk_mesh_pool.h"
#include "core/distance_queue.h"
#include "scene/scene_system.h"
#include "voxel/chunk_manager.h"
#include "voxel/voxel_bounds.h"
#include "voxel/voxel_constants.h"
#include "voxel_rendering/chunk_meshing_queue.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace h2o
{
    // class ChunkColumn;
    class INetPeer;
    class RenderingModule;
    class VoxelModule;

    class ChunkClient : public SceneSystem
    {
    public:

        explicit ChunkClient(
            const SceneSystemInitializer& system_initializer,
            INetPeer& client);
        ~ChunkClient() override = default;

        [[nodiscard]] ChunkManager& chunk_mgr() { return m_chunk_mgr; }

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        void request_chunk_loads();
        void trim_far_chunks();

        void rebuild_chunk_mesh(const v3i& chunk_pos);

    private:

        ChunkManager m_chunk_mgr{};

        ChunkMeshPool m_chunk_mesh_pool{};
        ChunkMeshingQueue m_chunk_meshing_queue{};

        bool m_refresh_chunk_requests = false;

        // Net event handles
        EventHandle m_on_connected_handle{};
        EventHandle m_on_fetched_chunk_handle{};
        EventHandle m_on_received_block_place_request{};

        // Local events
        EventHandle m_on_block_placed{};

        VoxelBounds m_voxel_bounds;
        v2i m_previous_player_chunk_col_pos{};

        INetPeer*        m_client           = nullptr;
        RenderingModule* m_rendering_module = nullptr;
        VoxelModule*     m_voxel_module     = nullptr;

    };
}