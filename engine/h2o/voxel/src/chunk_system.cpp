#include "voxel/chunk_system.h"

#include "voxel/chunk.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    ChunkSystem::ChunkSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {

    }

    WeakHandle<Chunk> ChunkSystem::get_chunk_at(const v3i& world_pos) const
    {
        if (!is_valid_pos(world_pos))
            return nullptr;

        return m_chunks[to_index(world_pos)];
    }

    constexpr size_t ChunkSystem::to_index(const v3i& world_pos)
    {
        const size_t index =
            world_pos.y * m_world_area +
            world_pos.z * m_world_size +
            world_pos.x;

        return index;
    }

    constexpr bool ChunkSystem::is_valid_pos(const v3i& world_pos)
    {
        return
            world_pos.x >= 0 && world_pos.x < m_world_size &&
            world_pos.z >= 0 && world_pos.z < m_world_size &&
            world_pos.y >= 0 && world_pos.y < m_world_height;
    }

    bool ChunkSystem::init()
    {
        for (i32 x = 0; x < m_world_size;   x++)
        for (i32 z = 0; z < m_world_size;   z++)
        for (i32 y = 0; y < m_world_height; y++)
        {
            const v3i chunk_pos { x, y, z };
            auto chunk = oup::make_observable_unique<Chunk>(*this, chunk_pos);
            on_chunk_created.broadcast({ *chunk });
            m_chunks.push_back(std::move(chunk));
        }

        for (const auto& chunk : m_chunks)
        {
            if (!chunk)
                continue;

            for (i32 i = 0; i < voxel_constants::chunk_size; i++)
                chunk->set_block_at({ i, i, i }, { 1, 0 });

            on_chunk_updated.broadcast({ *chunk });
        }

        return true;
    }
}