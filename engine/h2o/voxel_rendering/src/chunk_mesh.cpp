#include "voxel_rendering/chunk_mesh.h"

#include "rendering/renderer.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk.h"
#include "voxel/voxel_constants.h"
#include "voxel/direction.h"

#include <magic_enum.hpp>

namespace h2o
{
    struct BlockVertex
    {
        // Vertex position
        u64 x : 10;
        u64 y : 10;
        u64 z : 10;

        // Texture coord
        u64 u : 5;
        u64 v : 5;

        u64 tex_idx : 11;
        u64 face_idx : 3;

        // Current bit total: 54
    };

    ChunkMesh::ChunkMesh(gfx::IRenderer& renderer, const v3i& chunk_pos)
        : m_chunk_pos(chunk_pos)
    {
        m_vertex_array = renderer.create_vertex_array();
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

    void ChunkMesh::update(const Chunk& chunk)
    {
        std::vector<BlockVertex> vertices;

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
                    // Draw this face
                }
            }
        }
    }
}