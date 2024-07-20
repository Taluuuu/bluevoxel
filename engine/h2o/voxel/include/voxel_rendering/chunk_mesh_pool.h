#pragma once

#include "core/types.h"
#include "rendering/vertex_array.h"
#include "voxel_rendering/chunk_mesh.h"

#include <functional>
#include <glm/gtx/hash.hpp>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class RenderingModule;
    class VoxelBounds;
    class VoxelModule;

    struct ChunkMeshData
    {
        gfx::VertexArray vertex_array;
        u32 vertex_count = 0;
        v3i chunk_pos{};
    };

    /**
     * Thread safe chunk mesh pool wrapper
     */
    class ChunkMeshPool
    {
    public:

        ChunkMeshPool();

        void for_each_chunk_mesh(const std::function<void(const ChunkMeshData&)>& function) const;
        void update_meshes(const VoxelBounds& voxel_bounds);

        // Can be called from other threads
        void build_chunk_mesh(const ChunkView<v3i{3}>& chunk_view);

    private:

        [[nodiscard]] ChunkMeshData* get_or_reserve_chunk_mesh(const v3i& chunk_pos, const VoxelBounds& voxel_bounds);

    private:

        std::vector<ChunkMeshData> m_chunk_mesh_pool{};
        std::unordered_map<v3i, u32> m_chunk_mesh_indices{};

        // Chunk meshes waiting to be sent to the gpu
        std::queue<ChunkMesh> m_built_chunk_meshes{};
        mutable std::mutex m_built_chunk_meshes_mutex;

        VoxelModule*     const m_voxel_module = nullptr;
        RenderingModule* const m_rendering_module = nullptr;

    };
}
