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
    ChunkMesh::ChunkMesh(
        const Chunk::ViewType& chunk_view,
        const ChunkLighting::ViewType& lighting_view,
        const VoxelModule& voxel_module)
    {
        build_mesh(chunk_view, lighting_view, voxel_module);
    }

    void ChunkMesh::build_mesh(
        const Chunk::ViewType& chunk_view,
        const ChunkLighting::ViewType& lighting_view,
        const VoxelModule& voxel_module)
    {
        m_chunk_pos = chunk_view.center_cell_pos();
        m_vertices.clear();

        const Chunk* chunk = chunk_view.get(m_chunk_pos);
        if (!chunk)
            return;

        const auto append_side =
            [&](const v3i& pos,
                const std::vector<u32>& textures,
                const std::vector<BlockModel::Triangle>& side,
                const ChunkLightLevel light_level)
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

                        vertex.light_level = light_level.light;
                        vertex.sunlight_level = light_level.sunlight;

                        const auto temp = vertex.to_array();
                        for (const u32 data : temp)
                            m_vertices.push_back(data);
                    }
                }
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
            const auto model = voxel_module.get_model(block, texture_ids);
            if (!model)
                continue;

            assert(texture_ids);

            u8 dir_index = 0;
            voxel::Direction::for_each(
                [&](const voxel::Direction::Type dir)
                {
                    const v3i offset = voxel::to_vec3(dir);
                    const v3i adjacent_block_pos = pos + offset;

                    const Block adj_block = chunk_view.get_block_at(adjacent_block_pos, EViewRelativeTo::ViewCenter);
                    const auto adj_light_level = lighting_view.get_light_level(adjacent_block_pos, EViewRelativeTo::ViewCenter);

                    if (voxel_module.is_transparent(adj_block.id))
                        append_side(pos, *texture_ids, model->occluded_triangles_per_side[dir_index], adj_light_level);

                    dir_index++;
                }
            );

            const auto light_level = lighting_view.get_light_level(pos, EViewRelativeTo::ViewCenter);
            append_side(pos, *texture_ids, model->unoccluded_triangles, light_level);
        }
    }
}