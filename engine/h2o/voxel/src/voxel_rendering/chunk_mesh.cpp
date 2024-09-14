#include "voxel_rendering/chunk_mesh.h"

#include "rendering/buffer.h"
#include "rendering/rendering_module.h"
#include "voxel/chunk.h"
#include "voxel/voxel_constants.h"
#include "voxel/direction.h"
#include "voxel/block_model.h"
#include "voxel/voxel_module.h"

#include <array>

namespace h2o
{
    ChunkMesh::ChunkMesh(const ChunkMeshingView& chunk_view, const VoxelModule& voxel_module)
    {
        build_mesh(chunk_view, voxel_module);
    }

    void ChunkMesh::build_mesh(const ChunkMeshingView& chunk_view, const VoxelModule& voxel_module)
    {
        m_chunk_pos = chunk_view.center_chunk_pos();
        m_vertices.clear();

        const Chunk* chunk = chunk_view.get_chunk_at(m_chunk_pos);
        if (!chunk)
            return;

        const auto append_side =
            [&](const v3i& pos,
                const std::vector<u32>& textures,
                const std::vector<BlockModel::Triangle>& side)
            {
                for (const auto& triangle : side)
                {
                    for (BlockVertex vertex : triangle)
                    {
                        vertex.x += pos.x * 16;
                        vertex.y += pos.y * 16;
                        vertex.z += pos.z * 16;

                        // Left is texture index relative to all textures; right is the face index.
                        vertex.tex_idx = textures[vertex.tex_idx];

                        auto temp = vertex.to_array();
                        for (u32 data : temp)
                            m_vertices.push_back(data);
                    }
                }
            };

        const auto is_transparent =
            [&](const v3i& block_pos, voxel::Direction::Type direction) -> bool
            {
                const v3i offset = voxel::to_vec3(direction);
                const v3i adj_pos = block_pos + offset;

                if (const auto block = chunk_view.get_block_at(adj_pos, ViewRelativeTo::ViewCenter))
                    return voxel_module.is_transparent(block->id);

                return true;
            };

        for (i32 y = 0; y < voxel_constants::chunk_size; y++)
        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            const v3i pos { x, y, z };

            const Block block = chunk->get_block_at(pos);
            if (block == Block::Air)
                continue;

            const std::vector<u32>* texture_ids = nullptr;
            const auto* model = voxel_module.get_model(block.id, texture_ids);
            if (!model)
                continue;

            assert(texture_ids);

            u8 dir_index = 0;
            voxel::Direction::for_each(
                [&](voxel::Direction::Type dir)
                {
                    if (is_transparent(pos, dir))
                        append_side(pos, *texture_ids, model->occluded_triangles_per_side[dir_index]);

                    dir_index++;
                }
            );

            append_side(pos, *texture_ids, model->unoccluded_triangles);
        }
    }
}