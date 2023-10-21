#pragma once

#include "chunk_mesh_pool.h"
#include "chunk_manager_client.h"
#include "core/distance_queue.h"
#include "networking/client.h"
#include "scene/scene_system.h"
#include "voxel/block_container_interface.h"
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

    private:

        void request_chunk_loads();
        void trim_far_chunks();

        void build_chunk_meshes(i32 max_chunk_meshes, const v3& player_pos);
        void build_chunk_mesh_at(const v3i& chunk_pos);

        [[nodiscard]] bool is_in_range(v2i chunk_pos) const;

    private:

        // If there is an entry in the map, the chunk has been requested.
        ChunkManager_Client m_chunk_mgr;

        ChunkMeshPool m_chunk_mesh_pool{};
        std::vector<v3i> m_chunks_to_mesh{};
        std::mutex m_chunks_to_mesh_mutex{};

        bool m_refresh_chunk_requests = false;

        // Net events
        EventHandle m_on_connected_handle{};
        EventHandle m_on_fetched_chunk_handle{};
        EventHandle m_on_received_block_place_request{};

        // Local events
        EventHandle m_on_block_placed{};

        i32 m_view_distance = 8;
        i32 m_stay_loaded_distance = 3;

        v2i m_previous_player_chunk_col_pos{};

        Client*               m_client                 = nullptr;
        RenderingModule*      m_rendering_module       = nullptr;
        VoxelModule*          m_voxel_module           = nullptr;
        VoxelRenderingModule* m_voxel_rendering_module = nullptr;

    };
}