#include "voxel_rendering/chunk_mesh_pool.h"

#include "core/engine.h"
#include "rendering/buffer.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "voxel/chunk.h"

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
        auto& mesh_render_data = m_chunk_mesh_pool.emplace_back(chunk_pos);

        // First time init of the VAO here
        auto& vao = mesh_render_data.vertex_array;
        vao.attach_vertex_buffer(m_rendering_module->renderer().create_buffer_ptr(), 0, 0, 3 * sizeof(u32));
        vao.setup_attribute_int(0, 0, gfx::AttributeType::U32, 1, 0);
        vao.setup_attribute_int(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
        vao.setup_attribute_int(2, 0, gfx::AttributeType::U32, 1, 2 * sizeof(u32));

        return mesh_render_data;
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