#include "voxel_client/chunk_mesh_pool.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "voxel/voxel_bounds.h"
#include "voxel_rendering/voxel_rendering_module.h"

namespace h2o
{
    ChunkMeshPool::ChunkMeshPool()
        : m_voxel_rendering_module(&g_engine->get_module_checked<VoxelRenderingModule>())
        , m_rendering_module (&g_engine->get_module_checked<RenderingModule>())
    {}

    void ChunkMeshPool::for_each_chunk_mesh(const std::function<void(const ChunkMeshData&)>& function) const
    {

    }

    void ChunkMeshPool::build_chunk_mesh(const ChunkRegion& chunk_region)
    {
        assert(m_voxel_rendering_module);
        ChunkMesh chunk_mesh(chunk_region, *m_voxel_rendering_module);

        std::lock_guard lock { m_built_chunk_meshes_mutex };
        m_built_chunk_meshes.push(std::move(chunk_mesh));
    }

    void ChunkMeshPool::update_meshes(const VoxelBounds& voxel_bounds)
    {
        std::lock_guard lock { m_built_chunk_meshes_mutex };
        while (!m_built_chunk_meshes.empty())
        {
            auto& chunk_mesh = m_built_chunk_meshes.front();

            auto it = m_chunk_mesh_indices.find(chunk_mesh.chunk_pos());
            if (it == m_chunk_mesh_indices.end())
            {

            }

            chunk_mesh.update_buffer();

            m_built_chunk_meshes.pop();
        }
    }

    ChunkMeshData& ChunkMeshPool::reserve_chunk_mesh(const v3i& chunk_pos, const VoxelBounds& voxel_bounds)
    {
        const v2i chunk_column_pos { chunk_pos.x, chunk_pos.z };
        if (!voxel_bounds.)
    }

//    ChunkMesh& ChunkMeshPool::find_or_create_chunk_mesh(const v3i& chunk_pos)
//    {
//        if (ChunkMeshData* mesh_data = find_mesh(chunk_pos))
//            return mesh_data->chunk_mesh;
//
//        return create_mesh(chunk_pos).chunk_mesh;
//    }
//
//    ChunkMeshData& ChunkMeshPool::create_mesh(const v3i& chunk_pos)
//    {
//        assert(find_mesh(chunk_pos) == nullptr);
//
//        auto [mesh_data, mesh_id] = reserve_chunk_mesh();
//        assign_chunk_mesh(chunk_pos, mesh_id);
//
//        assert(m_voxel_rendering_module && m_rendering_module);
//        mesh_data.chunk_mesh.init(*m_voxel_rendering_module, *m_rendering_module);
//
//        return mesh_data;
//    }
//
//    std::pair<ChunkMeshData&, ChunkMeshID> ChunkMeshPool::reserve_chunk_mesh()
//    {
//        i32 index = 0;
//        for (; index < m_chunk_mesh_pool.size(); index++)
//        {
//            auto& chunk_mesh_data = m_chunk_mesh_pool[index];
//            if (chunk_mesh_data.is_available)
//            {
//                chunk_mesh_data.is_available = false;
//                return { chunk_mesh_data, index };
//            }
//        }
//
//        auto& chunk_mesh_data = m_chunk_mesh_pool.emplace_back();
//        chunk_mesh_data.is_available = false;
//
//        return { chunk_mesh_data, index };
//    }
//
//    void ChunkMeshPool::assign_chunk_mesh(const v3i& chunk_pos, ChunkMeshID mesh_id)
//    {
//        m_chunk_mesh_indices[chunk_pos] = mesh_id;
//    }
//
//    ChunkMeshData* ChunkMeshPool::find_mesh(const v3i& chunk_pos)
//    {
//        if (auto mesh_id = find_mesh_id(chunk_pos))
//        {
//            assert(*mesh_id < m_chunk_mesh_pool.size());
//            return &m_chunk_mesh_pool[*mesh_id];
//        }
//
//        return nullptr;
//    }
//
//    std::optional<ChunkMeshID> ChunkMeshPool::find_mesh_id(const v3i& chunk_pos) const
//    {
//        if (auto it = m_chunk_mesh_indices.find(chunk_pos); it != m_chunk_mesh_indices.end())
//            return it->second;
//
//        return std::nullopt;
//    }
}