#pragma once

#include "core/types.h"
#include "voxel_rendering/chunk_mesh.h"

#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <vector>

namespace h2o
{
    // Not thread safe
    // An auto resizing pool of chunk meshes
    class ChunkMeshPool
    {
    public:

        ChunkMeshPool() = default;

        // Iterate through all valid chunk meshes
        void for_each_chunk_mesh(const std::function<void(const ChunkMeshRenderData&)>& function) const;

        // Fetch the existing chunk mesh associated with the input chunk pos,
        // or create one if one does not yet exist.
        ChunkMeshRenderData& fetch_or_create_mesh(const v3i& chunk_pos);

        // Stop drawing this chunk mesh and make it available for another chunk.
        void free_mesh(const v3i& chunk_pos);

    private:

        std::vector<ChunkMeshRenderData> m_chunk_mesh_pool{};
        std::unordered_map<v3i, u32> m_chunk_mesh_indices{};

    };
}
