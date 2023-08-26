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
//    ChunkMesh::ChunkMesh(ChunkMesh&& other) noexcept
//        : m_chunk_pos(other.m_chunk_pos)
//        , m_vertex_array(std::move(other.m_vertex_array))
//        , m_buffer(std::move(other.m_buffer))
//    {}
//
//    ChunkMesh& ChunkMesh::operator=(ChunkMesh&& other) noexcept
//    {
//        if (this != &other)
//        {
//            m_chunk_pos = other.m_chunk_pos;
//            m_vertex_array = std::move(other.m_vertex_array);
//            m_buffer = std::move(other.m_buffer);
//        }
//
//        return *this;
//    }

    void ChunkMesh::init(
        const VoxelRenderingModule& voxel_rendering_module,
        gfx::IRenderer& renderer,
        const v3i& chunk_pos)
    {
        assert(!m_vertex_array && !m_buffer);

        m_chunk_pos = chunk_pos;

        m_vertex_array = renderer.create_vertex_array();
        m_buffer = renderer.create_buffer();

        m_voxel_rendering_module = &voxel_rendering_module;
    }

    void ChunkMesh::update(
        const Chunk& chunk,
        const std::array<Chunk*, 6>& adjacent_chunks)
    {
        assert(m_vertex_array);
        assert(m_buffer);
        assert(m_voxel_rendering_module);

        // TODO: Investigate the possibility of storing an array of u8 describing adjacent faces
        //       that need to be added to the mesh. This array could be updated when we edit blocks

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
                    vertices.push_back(temp[0]);
                    vertices.push_back(temp[1]);
                }
            };

        const auto get_adj_block_at =
            [&chunk, &adjacent_chunks](const v3i& block_pos, u32 direction) -> Block
            {
                const v3i offset = voxel::to_vec3(direction);
                const v3i adj_pos = block_pos + offset;

                if (Chunk::is_valid_pos(adj_pos))
                    return chunk.get_block_at(adj_pos);

                if (Chunk* adj_chunk = adjacent_chunks[direction])
                {
                    const v3i pos_in_chunk = block_pos_to_within_chunk(adj_pos);
                    return adj_chunk->get_block_at(pos_in_chunk);
                }

                return Block::Air;
            };

        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 y = 0; y < voxel_constants::chunk_size; y++)
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

            for (u32 dir = 0; dir < voxel::dir_count; dir++)
            {
                if (get_adj_block_at(pos, dir) == Block::Air)
                {
                    for (const auto& face : model->occluded_vertices[dir])
                        append_face(pos, *model, textures, face);
                }
            }

            for (const auto& face : model->unoccluded_vertices)
                append_face(pos, *model, textures, face);
        }

        if (m_vertex_count == 0)
        {
            m_vertex_array->attach_vertex_buffer(m_buffer, 0, 0, 2 * sizeof(u32));
            m_vertex_array->setup_attribute(0, 0, gfx::AttributeType::U32, 1, 0);
            m_vertex_array->setup_attribute(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
        }

        m_vertex_count = static_cast<i32>(vertices.size() / 2);

        m_buffer->update_data(vertices.data(), vertices.size() * sizeof(u32));
    }

    const gfx::IVertexArray& ChunkMesh::vertex_array() const
    {
        // TODO: Find a better way to check for vao validity
        assert(m_vertex_array);
        return *m_vertex_array;
    }
}