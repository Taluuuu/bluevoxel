#include "voxel_rendering/chunk_mesh.h"

#include "core/log.h"
#include "rendering/renderer.h"
#include "rendering/buffer.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"
#include "voxel/direction.h"
#include "voxel/voxel_utils.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "voxel_rendering/block_model.h"

#include <array>
#include <magic_enum.hpp>

namespace h2o
{
    void ChunkMesh::init(
        const VoxelRenderingModule& voxel_rendering_module,
        gfx::IRenderer& renderer)
    {
        assert(!m_vertex_array && !m_buffer);

        m_vertex_array = renderer.create_vertex_array();
        m_buffer = renderer.create_buffer();

        m_voxel_rendering_module = &voxel_rendering_module;
    }

    void ChunkMesh::generate_vertices(const ChunkRegion& chunk_region)
    {
        assert(m_vertex_array);
        assert(m_buffer);
        assert(m_voxel_rendering_module);

        m_chunk_pos = chunk_region.center_chunk_pos();
        const Chunk* chunk = chunk_region.get_chunk_at(m_chunk_pos);
        assert(chunk);

        std::lock_guard lock { m_vertices_mutex };
        m_pending_vertices.clear();

        const auto append_face =
            [&](
                const v3i& pos,
                const BlockModel& model,
                const std::vector<u32>& textures,
                const std::vector<BlockVertex>& face)
            {
                for (BlockVertex vertex : face)
                {
                    vertex.x += pos.x * 8;
                    vertex.y += pos.y * 8;
                    vertex.z += pos.z * 8;

                    vertex.tex_idx = textures[vertex.tex_idx];

                    auto temp = vertex.to_array();
                    for (u32 data : temp)
                        m_pending_vertices.push_back(data);
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

            const BlockModel* model = m_voxel_rendering_module->get_model_fast(block.id);
            if (!model)
                continue;

            const auto& textures = m_voxel_rendering_module->get_textures_fast(block.id);

            u8 dir_index = 0;
            magic_enum::enum_for_each<voxel::Direction::Type>(
                [&](voxel::Direction::Type dir)
                {
                    if (get_adj_block_at(pos, dir) == Block::Air)
                    {
                        for (const auto& face : model->occluded_vertices[dir_index])
                            append_face(pos, *model, textures, face);
                    }

                    dir_index++;
                }
            );

            for (const auto& face : model->unoccluded_vertices)
                append_face(pos, *model, textures, face);
        }
    }

    void ChunkMesh::update_mesh()
    {
        if (m_vertex_count == 0)
        {
            m_vertex_array->attach_vertex_buffer(m_buffer, 0, 0, 3 * sizeof(u32));
            m_vertex_array->setup_attribute(0, 0, gfx::AttributeType::U32, 1, 0);
            m_vertex_array->setup_attribute(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
            m_vertex_array->setup_attribute(2, 0, gfx::AttributeType::U32, 1, 2 * sizeof(u32));
        }

        m_vertex_count = static_cast<i32>(m_pending_vertices.size() / 3);

        m_buffer->update_data(m_pending_vertices.data(), i32(m_pending_vertices.size() * sizeof(u32)));

        m_pending_vertices.clear();
    }

    const gfx::IVertexArray& ChunkMesh::vertex_array() const
    {
        // TODO: Find a better way to check for vao validity
        assert(m_vertex_array);
        return *m_vertex_array;
    }
}