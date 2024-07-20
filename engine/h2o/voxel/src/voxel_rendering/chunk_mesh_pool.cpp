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
    ChunkMeshPool::ChunkMeshPool()
        : m_voxel_module(&g_engine->get_module_checked<VoxelModule>())
        , m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
    {}

    void ChunkMeshPool::for_each_chunk_mesh(const std::function<void(const ChunkMeshData&)>& function) const
    {
        for (const ChunkMeshData& mesh_data : m_chunk_mesh_pool)
            function(mesh_data);
    }

    void ChunkMeshPool::update_meshes(const VoxelBounds& voxel_bounds)
    {
        std::lock_guard lock { m_built_chunk_meshes_mutex };
        while (!m_built_chunk_meshes.empty())
        {
            auto& chunk_mesh = m_built_chunk_meshes.front();
            if (auto mesh_data = get_or_reserve_chunk_mesh(chunk_mesh.chunk_pos(), voxel_bounds))
            {
                assert(mesh_data->vertex_array.get_vertex_buffer(0));

                const auto& vertices = chunk_mesh.vertices();
                mesh_data->vertex_array.get_vertex_buffer(0)->update_data(
                    vertices.data(), vertices.size() * sizeof(u32), gfx::BufferUsage::StaticDraw);

                mesh_data->vertex_count = chunk_mesh.vertex_count();
            }

            m_built_chunk_meshes.pop();
        }
    }

    void ChunkMeshPool::build_chunk_mesh(const ChunkView<v3i{3}>& chunk_view)
    {
        assert(m_voxel_module);

        // TODO: Could chunk->is_empty() here cause a problem when destroying the last block of a chunk ?
        const Chunk* chunk = chunk_view.get_chunk_at(v3i{ 0 }, ViewRelativeTo::ViewCenter);
        if (chunk && !chunk->is_empty())
        {
            // This builds the mesh, but is not that explicit. Could be reworked.
            ChunkMesh chunk_mesh(chunk_view, *m_voxel_module);

            std::lock_guard lock { m_built_chunk_meshes_mutex };
            m_built_chunk_meshes.push(std::move(chunk_mesh));
        }
    }

    ChunkMeshData* ChunkMeshPool::get_or_reserve_chunk_mesh(const v3i& chunk_pos, const VoxelBounds& voxel_bounds)
    {
        if (!voxel_bounds.in_bounds({ chunk_pos.x, chunk_pos.z }))
            return nullptr;

        if (const auto it = m_chunk_mesh_indices.find(chunk_pos); it != m_chunk_mesh_indices.end())
            return &m_chunk_mesh_pool[it->second];

        for (const auto [other_chunk_pos, index] : m_chunk_mesh_indices)
        {
            if (!voxel_bounds.in_bounds({ other_chunk_pos.x, other_chunk_pos.z }))
            {
                // Reassign this chunk mesh
                m_chunk_mesh_indices.erase(other_chunk_pos);
                m_chunk_mesh_indices[chunk_pos] = index;

                auto& mesh_data = m_chunk_mesh_pool[index];
                mesh_data.chunk_pos = chunk_pos;
                return &mesh_data;
            }
        }

        assert(m_rendering_module);
        auto& renderer = m_rendering_module->renderer();

        // Create a new chunk mesh.
        const u32 index = m_chunk_mesh_pool.size();
        auto& chunk_mesh = m_chunk_mesh_pool.emplace_back(
            renderer.create_vertex_array(),
            0,
            chunk_pos);

        // Init vertex array
        auto& vao = chunk_mesh.vertex_array;
        vao.attach_vertex_buffer(renderer.create_buffer_ptr(), 0, 0, 3 * sizeof(u32));
        vao.setup_attribute_int(0, 0, gfx::AttributeType::U32, 1, 0);
        vao.setup_attribute_int(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
        vao.setup_attribute_int(2, 0, gfx::AttributeType::U32, 1, 2 * sizeof(u32));

        m_chunk_mesh_indices[chunk_pos] = index;

        return &chunk_mesh;
    }
}