#pragma once

#include "core/types.h"
#include "voxel_rendering/chunk_mesh.h"

#include <functional>
#include <glm/gtx/hash.hpp>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

namespace h2o
{
    struct ChunkMeshData
    {
        ChunkMesh chunk_mesh{};
        bool is_available = true;
    };

    using ChunkMeshID = u32;

    /**
     * Thread safe chunk mesh pool wrapper
     */
    class ChunkMeshPool
    {
    public:

        void fetch_or_create_chunk_mesh(const v3i& chunk_pos, const std::function<void(ChunkMesh&)>& function);
        void for_each_chunk_mesh(const std::function<void(const ChunkMesh&)>& function) const;

    private:

        [[nodiscard]] ChunkMeshData& create_mesh(const v3i& chunk_pos);
        [[nodiscard]] std::pair<ChunkMeshData&, ChunkMeshID> reserve_chunk_mesh();
        void assign_chunk_mesh(const v3i& chunk_pos, ChunkMeshID mesh_id);

        [[nodiscard]] ChunkMeshData* find_mesh(const v3i& chunk_pos);
        [[nodiscard]] std::optional<ChunkMeshID> find_mesh_id(const v3i& chunk_pos) const;

    private:

        std::vector<ChunkMeshData> m_chunk_mesh_pool{};
        std::unordered_map<v3i, ChunkMeshID> m_chunk_mesh_indices{};
        mutable std::mutex m_mutex;

    };
}