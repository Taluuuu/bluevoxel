#include "voxel/chunk_mgr.h"

#include "voxel/chunk.h"

namespace h2o
{
    ChunkMgr::ChunkMgr(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        const u32 num_chunks = m_world_size * m_world_size;
        for (u32 i = 0; i < num_chunks; i++)
        {
            m_chunks.push_back(std::make_unique<Chunk>(*this));
        }
    }

    Chunk* ChunkMgr::get_chunk_at(const v3i& world_pos) const
    {
        if (!is_valid_pos(world_pos))
            return nullptr;

        return m_chunks[to_index(world_pos)].get();
    }

    const std::vector<ChunkPtr>& ChunkMgr::get_chunks() const
    {
        return m_chunks;
    }

    constexpr size_t ChunkMgr::to_index(const v3i& world_pos)
    {
        const size_t index =
            world_pos.y * m_world_area +
            world_pos.z * m_world_size +
            world_pos.x;

        return index;
    }

    constexpr bool ChunkMgr::is_valid_pos(const v3i& world_pos)
    {
        return
            world_pos.x >= 0 && world_pos.x < m_world_size &&
            world_pos.z >= 0 && world_pos.z < m_world_size &&
            world_pos.y >= 0 && world_pos.y < m_world_height;
    }
}