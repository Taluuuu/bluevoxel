#include "voxel_rendering/chunk_mesh_pool.h"

#include <voxel/chunk.h>

#include "core/engine.h"
#include "rendering/buffer.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "voxel/chunk_view.h"
#include "voxel/voxel_bounds.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    void ChunkMeshPool::for_each_chunk_mesh(const std::function<void(const ChunkMeshRenderData&)>& function) const
    {
        for (const ChunkMeshRenderData& mesh_data : m_chunk_mesh_pool)
            function(mesh_data);
    }

    ChunkMeshRenderData& ChunkMeshPool::fetch_or_create_mesh(const v3i& chunk_pos)
    {
        if (const auto it = m_chunk_mesh_indices.find(chunk_pos); it != m_chunk_mesh_indices.end())
            return m_chunk_mesh_pool[it->second];

        m_chunk_mesh_indices.emplace(chunk_pos, m_chunk_mesh_pool.size());
        return m_chunk_mesh_pool.emplace_back();
    }

    void ChunkMeshPool::free_mesh(const v3i& chunk_pos)
    {
        if (const auto it = m_chunk_mesh_indices.find(chunk_pos); it != m_chunk_mesh_indices.end())
        {
            // TODO: Could this swap the removed element with the last one ?
            std::erase(m_chunk_mesh_pool, m_chunk_mesh_pool.begin() + it->second);
            m_chunk_mesh_indices.erase(it);
        }
    }
}