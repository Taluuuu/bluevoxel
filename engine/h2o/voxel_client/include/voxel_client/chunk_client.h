#pragma once

#include "core/distance_queue.h"
#include "networking/client.h"
#include "scene/scene_system.h"
#include "voxel/voxel_constants.h"
#include "voxel_rendering/chunk_mesh.h"

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

        // Tickable interface
        void update(f32 delta_time) override;
        void render(f32 delta_time) override;

    private:

        void request_chunk_loads();
        void trim_far_chunks();

        void build_chunk_meshes(i32 max_chunk_meshes, const v3& player_pos);
        void build_chunk_mesh_at(const v3i& chunk_pos);

        struct ChunkData
        {
            ChunkData()
            { chunk_mesh_indices.fill(-1); }

            std::shared_ptr<ChunkColumn> chunk_column = nullptr;
            std::array<i32, voxel_constants::vertical_chunk_count> chunk_mesh_indices{};
        };

        struct ChunkMeshData
        {
            ChunkMesh chunk_mesh{};
            bool is_available = true;
        };

        [[nodiscard]] bool is_in_range(v2i chunk_pos) const;
        [[nodiscard]] std::pair<ChunkMeshData&, i32> reserve_chunk_mesh();

    private:

        // If there is an entry in the map, the chunk has been requested.
        std::mutex m_chunk_columns_mutex{};
        std::unordered_map<v2i, ChunkData> m_chunk_columns{};
        std::vector<ChunkMeshData> m_chunk_mesh_pool{};

        std::vector<v3i> m_chunks_to_mesh{};
        std::mutex m_chunks_to_mesh_mutex{};

        Client* m_client = nullptr;
        bool m_refresh_chunk_requests = false;

        EventHandle m_on_connected_handle{};
        EventHandle m_on_fetched_chunk_handle{};

        i32 m_view_distance = 8;
        i32 m_stay_loaded_distance = 3;

        v2i m_previous_player_chunk_col_pos{};

        RenderingModule*      m_rendering_module       = nullptr;
        VoxelModule*          m_voxel_module           = nullptr;
        VoxelRenderingModule* m_voxel_rendering_module = nullptr;

    };
}