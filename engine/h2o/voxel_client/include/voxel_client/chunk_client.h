#pragma once

#include "chunk_mesh_pool.h"
#include "chunk_manager_client.h"
#include "core/distance_queue.h"
#include "networking/client.h"
#include "scene/scene_system.h"
#include "voxel/chunk_container_interface.h"
#include "voxel/chunk_manager_interface.h"
#include "voxel/voxel_constants.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace h2o
{
    class ChunkColumn;
    class RenderingModule;
    class VoxelRenderingModule;
    class VoxelModule;

    class ChunkClient : public SceneSystem
    {
    public:

        explicit ChunkClient(
            const SceneSystemInitializer& system_initializer,
            Client& client);
        ~ChunkClient() override = default;

        [[nodiscard]] ChunkManager_Client& chunk_mgr() { return m_chunk_mgr; }

        // Tickable interface
        void update(f32 delta_time) override;
        void render() override;

    public:

        v3 light_dir { 0.18f, -1.0f, 0.492f };
        v3 light_color { 1.0f, 1.0f, 1.0f };
        f32 ambient_strength = 0.714f;

    private:

        void request_chunk_loads();
        void trim_far_chunks();

        void rebuild_chunk_mesh(const v3i& chunk_pos);

        [[nodiscard]] bool is_in_range(v2i chunk_pos) const;

    private:

        // If there is an entry in the map, the chunk has been requested.
        ChunkManager_Client m_chunk_mgr;

        ChunkMeshPool m_chunk_mesh_pool{};

        bool m_refresh_chunk_requests = false;

        // Net event handles
        EventHandle m_on_connected_handle{};
        EventHandle m_on_fetched_chunk_handle{};
        EventHandle m_on_received_block_place_request{};

        // Local events
        EventHandle m_on_block_placed{};

        i32 m_view_distance = 8;
        i32 m_stay_loaded_distance = 3;

        std::atomic_int m_num_requested_chunks = 0;
        std::atomic_int m_num_received_chunks = 0;

        v2i m_previous_player_chunk_col_pos{};

        Client*               m_client                 = nullptr;
        RenderingModule*      m_rendering_module       = nullptr;
        VoxelRenderingModule* m_voxel_rendering_module = nullptr;

    };
}