#include "voxel/chunk_column.h"

#include "core/engine.h"

namespace h2o
{
    ChunkColumn::ChunkColumn(v2i chunk_col_pos)
        : m_chunk_col_pos(chunk_col_pos)
    {
        auto& voxel_module = g_engine->get_module_checked<VoxelModule>();

        i32 y = 0;
        for (auto& chunk : m_chunks)
        {
            chunk.m_chunk_pos = { chunk_col_pos.x, y++, chunk_col_pos.y };
            chunk.init(voxel_module);
        }
    }

    void ChunkColumn::increment_generation_stage()
    {
        m_generation_stage++;
    }

    void ChunkColumn::finish_generation()
    {
        m_generation_stage = voxel_constants::max_generation_stage;
    }

    void ChunkColumn::tick()
    {
        assert(is_generated());
        for (auto& chunk : m_chunks)
            chunk.tick();
    }

    Chunk* ChunkColumn::get_chunk_safe(i32 y)
    {
        if (y >= 0 && y < m_chunks.size())
            return &m_chunks[y];

        return nullptr;
    }

    const Chunk* ChunkColumn::get_chunk_safe(i32 y) const
    {
        if (y >= 0 && y < m_chunks.size())
            return &m_chunks[y];

        return nullptr;
    }
}