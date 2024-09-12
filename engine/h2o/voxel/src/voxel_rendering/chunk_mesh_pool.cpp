#include "voxel_rendering/chunk_mesh_pool.h"

#include <voxel/chunk.h>

#include "core/engine.h"
#include "rendering/buffer.h"
#include "rendering/rendering_module.h"

namespace h2o
{
    ChunkMeshPool::ChunkMeshPool()
        : m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
    {}

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
        return m_chunk_mesh_pool.emplace_back(chunk_pos);
    }

    void ChunkMeshPool::free_mesh(const v3i& chunk_pos)
    {
        if (const auto it = m_chunk_mesh_indices.find(chunk_pos); it != m_chunk_mesh_indices.end())
        {
            // TODO: Swap the last element in place of the deleted mesh instead
            const u32 mesh_index_to_delete = it->second;
            m_chunk_mesh_pool.erase(m_chunk_mesh_pool.begin() + mesh_index_to_delete);
            m_chunk_mesh_indices.erase(it);

            // Preserve indices
            for (auto& [_, mesh_index] : m_chunk_mesh_indices)
            {
                if (mesh_index > mesh_index_to_delete)
                    mesh_index--;
            }
        }
    }
}