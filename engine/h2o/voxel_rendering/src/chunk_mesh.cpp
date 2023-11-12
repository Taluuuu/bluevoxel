#include "voxel_rendering/chunk_mesh.h"

#include "core/log.h"
#include "rendering/renderer.h"
#include "rendering/buffer.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk.h"
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

    void ChunkMesh::update(const Chunk& chunk)
    {
        assert(m_vertex_array);
        assert(m_buffer);
        assert(m_voxel_rendering_module);

        m_chunk_pos = chunk.chunk_pos();

        std::vector<u32> vertices;

        const auto append_face =
            [&vertices](
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
                        vertices.push_back(data);
                }
            };

        for (i32 y = 0; y < voxel_constants::chunk_size; y++)
        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            const v3i pos { x, y, z };

            const Block block = chunk.get_block_at(pos);
            if (block == Block::Air)
                continue;

            const BlockModel* model = m_voxel_rendering_module->get_model_fast(block.id);
            if (!model)
                continue;

            const auto& textures = m_voxel_rendering_module->get_textures_fast(block.id);

            const auto adj_blocks = chunk.get_adjacent_blocks(pos);
            u8 dir_index = 0;
            magic_enum::enum_for_each<voxel::Direction::Type>(
                [&](voxel::Direction::Type dir)
                {
                    if (!(adj_blocks & dir))
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

        if (m_vertex_count == 0)
        {
            m_vertex_array->attach_vertex_buffer(m_buffer, 0, 0, 3 * sizeof(u32));
            m_vertex_array->setup_attribute(0, 0, gfx::AttributeType::U32, 1, 0);
            m_vertex_array->setup_attribute(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
            m_vertex_array->setup_attribute(2, 0, gfx::AttributeType::U32, 1, 2 * sizeof(u32));
        }

        m_vertex_count = static_cast<i32>(vertices.size() / 3);

        m_buffer->update_data(vertices.data(), i32(vertices.size() * sizeof(u32)));
    }

    const gfx::IVertexArray& ChunkMesh::vertex_array() const
    {
        // TODO: Find a better way to check for vao validity
        assert(m_vertex_array);
        return *m_vertex_array;
    }
}