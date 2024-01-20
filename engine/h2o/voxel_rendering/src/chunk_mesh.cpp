#include "voxel_rendering/chunk_mesh.h"

#include "core/log.h"
#include "rendering/buffer.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"
#include "voxel/direction.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "voxel/block_model.h"

#include <array>
#include <magic_enum.hpp>

namespace h2o
{
    ChunkMesh::ChunkMesh(const ChunkRegion& chunk_region, const VoxelRenderingModule& voxel_rendering_module)
    {
        build_mesh(chunk_region, voxel_rendering_module);
    }

    void ChunkMesh::build_mesh(const ChunkRegion& chunk_region, const VoxelRenderingModule& voxel_rendering_module)
    {
        m_chunk_pos = chunk_region.center_chunk_pos();
        const Chunk* chunk = chunk_region.get_chunk_at(m_chunk_pos);
        assert(chunk);

        m_vertices.clear();

        const auto append_face =
            [&](const v3i& pos,
                const BlockModel& model,
                const std::vector<u32>& textures,
                const BlockModel::Face& face)
            {
                for (const auto& triangle : face)
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

        const auto get_adj_block_at =
            [&](const v3i& block_pos, voxel::Direction::Type direction) -> Block
            {
                const v3i offset = voxel::to_vec3(direction);
                const v3i adj_pos = block_pos + offset;

                if (const auto block = chunk_region.get_block_at(adj_pos, m_chunk_pos))
                    return *block;

                return Block::Air;
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
            const auto* model = voxel_rendering_module.get_model(block.id, texture_ids);
            if (!model)
                continue;

            assert(texture_ids);

            u8 dir_index = 0;
            magic_enum::enum_for_each<voxel::Direction::Type>(
                [&](voxel::Direction::Type dir)
                {
                    if (get_adj_block_at(pos, dir) == Block::Air)
                    {
                        for (const auto& face : model->occluded_faces_per_side[dir_index])
                            append_face(pos, *model, *texture_ids, face);
                    }

                    dir_index++;
                }
            );

            for (const auto& face : model->unoccluded_faces)
                append_face(pos, *model, *texture_ids, face);
        }
    }
}