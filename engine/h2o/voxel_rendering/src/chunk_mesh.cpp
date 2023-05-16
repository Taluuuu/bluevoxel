#include "voxel_rendering/chunk_mesh.h"

#include "core/log.h"
#include "rendering/renderer.h"
#include "rendering/buffer.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk.h"
#include "voxel/voxel_constants.h"
#include "voxel/direction.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "voxel_rendering/block_model.h"

#include <array>
#include <magic_enum.hpp>

namespace h2o
{
    ChunkMesh::ChunkMesh(gfx::IRenderer& renderer, const v3i& chunk_pos)
        : m_chunk_pos(chunk_pos)
    {
        m_vertex_array = renderer.create_vertex_array();
        m_buffer = renderer.create_buffer();
    }

    ChunkMesh::ChunkMesh(ChunkMesh&& other) noexcept
        : m_chunk_pos(other.m_chunk_pos)
        , m_vertex_array(std::move(other.m_vertex_array))
        , m_buffer(std::move(other.m_buffer))
    {}

    ChunkMesh& ChunkMesh::operator=(ChunkMesh&& other) noexcept
    {
        if (this != &other)
        {
            m_chunk_pos = other.m_chunk_pos;
            m_vertex_array = std::move(other.m_vertex_array);
            m_buffer = std::move(other.m_buffer);
        }

        return *this;
    }

    void ChunkMesh::update(const VoxelRenderingModule& chunk_rendering_module, const Chunk& chunk)
    {
        std::vector<u32> vertices;
        auto model = chunk_rendering_module.get_model("cube");
        if (!model)
        {
            log::error("No cube model found :/");
            return;
        }

        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 y = 0; y < voxel_constants::chunk_size; y++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            const v3i pos { x, y, z };

            const Block block = chunk.get_block_at(pos);
            if (block == Block::Air)
                continue;

            for (u32 dir = 0; dir < magic_enum::enum_count<voxel::Direction>(); dir++)
            {
                const v3i offset = voxel::to_vec(dir);
                const Block neighbor_block = chunk.get_block_at(offset + pos);

                if (neighbor_block == Block::Air)
                {
                    for (auto vertex : model->occluded_vertices[dir])
                    {
                        vertex.x += x * 8;
                        vertex.y += y * 8;
                        vertex.z += z * 8;

                        auto temp = vertex.to_array();
                        vertices.push_back(temp[0]);
                        vertices.push_back(temp[1]);
                    }
                }

                for (auto vertex : model->unoccluded_vertices[dir])
                {
                    vertex.x += x * 8;
                    vertex.y += y * 8;
                    vertex.z += z * 8;

                    auto temp = vertex.to_array();
                    vertices.push_back(temp[0]);
                    vertices.push_back(temp[1]);
                }
            }
        }

        m_vertex_count = static_cast<i32>(vertices.size() / 2);

        m_buffer->update_data(vertices.data(), vertices.size() * sizeof(u32));
        m_vertex_array->attach_vertex_buffer(m_buffer, 0, 0, 2 * sizeof(u32));
        m_vertex_array->setup_attribute(0, 0, gfx::AttributeType::U32, 1, 0);
        m_vertex_array->setup_attribute(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
    }

    const gfx::IVertexArray& ChunkMesh::vertex_array() const
    {
        // TODO: Find a better way to check for vao validity
        assert(m_vertex_array);
        return *m_vertex_array;
    }
}