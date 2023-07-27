#include "voxel/chunk_region.h"

#include "voxel/chunk.h"
#include "voxel/chunk_system.h"

#include <cassert>
#include <optional>

namespace h2o
{
    ChunkRegion::ChunkRegion(const WeakHandle<ChunkSystem>& chunk_system)
        : m_chunk_system(chunk_system)
    {}

    void ChunkRegion::set_corner_pos(v2i new_corner_pos)
    {
        update(new_corner_pos, m_size);
    }

    void ChunkRegion::set_size(u32 new_size)
    {
        update(m_corner_pos, new_size);
    }

    void ChunkRegion::update(v2i new_corner_pos, u32 new_size)
    {
        assert(m_chunk_system);

        const auto new_to_old_indices = gen_new_to_old_indices(new_corner_pos, new_size);

        m_corner_pos = new_corner_pos;
        m_size = new_size;

        std::vector<v2i> rel_chunk_positions_to_load;
        rel_chunk_positions_to_load.reserve(new_size * new_size);

        std::vector<ChunkColumnPtr> new_chunks(new_size * new_size, nullptr);
        for (i32 i = 0; i < new_size; i++)
            for (i32 j = 0; j < new_size; j++)
            {
                const i32 new_idx = i * i32(new_size) + j;
                const i32 old_idx = new_to_old_indices[new_idx];

                if (old_idx != -1)
                {
                    new_chunks[new_idx] = m_chunks_in_region[old_idx];
                }
                else
                {
                    rel_chunk_positions_to_load.emplace_back(i, j);
                }
            }

        m_chunks_in_region = std::move(new_chunks);

        on_indices_changed(new_to_old_indices);

        for (v2i rel_chunk_pos : rel_chunk_positions_to_load)
        {
            const v2i world_chunk_pos = rel_chunk_pos + new_corner_pos;
            m_chunk_system->fetch_or_create_chunk_column(world_chunk_pos,
                [&, world_chunk_pos](const ChunkColumnPtr& chunk_col)
                {
                    if (in_region_bounds(world_chunk_pos))
                    {
                        assert(chunk_col && !chunk_col->empty());

                        // Recompute local chunk pos as the size or corner of the chunk region
                        // might have changed by the time we receive the new chunk.
                        const v2i local_chunk_pos = to_local_chunk_pos_2d(world_chunk_pos);
                        m_chunks_in_region[to_index(local_chunk_pos)] = chunk_col;
                        on_chunk_fetched(chunk_col, local_chunk_pos);
                    }
                });
        }
    }

    void ChunkRegion::for_each_chunk(const std::function<void(Chunk&)>& fun) const
    {
        for (const auto& chunk_col : m_chunks_in_region)
        {
            if (chunk_col)
            {
                for (auto& chunk : *chunk_col)
                    fun(chunk);
            }
        }
    }

    Chunk* ChunkRegion::get_chunk_at(const v3i& chunk_pos) const
    {
        if (!in_region_bounds(chunk_pos))
            return nullptr;

        const v3i local_pos = to_local_chunk_pos_3d(chunk_pos);
        const size_t index = to_index({ local_pos.x, local_pos.z });
        const auto& chunk_col = m_chunks_in_region[index];
        if (!chunk_col)
            return nullptr;

        return &(*chunk_col)[local_pos.y];
    }

    bool ChunkRegion::in_region_bounds(const v3i& chunk_pos) const
    {
        const v3i min { m_corner_pos.x, 0, m_corner_pos.y };
        const v3i max = min + v3i{ m_size, voxel_constants::vertical_chunk_count, m_size };

        return
            chunk_pos.x >= min.x && chunk_pos.x < max.x &&
            chunk_pos.y >= min.y && chunk_pos.y < max.y &&
            chunk_pos.z >= min.z && chunk_pos.z < max.z;
    }

    bool ChunkRegion::in_region_bounds(v2i chunk_pos) const
    {
        const v2i min = m_corner_pos;
        const v2i max = min + v2i { m_size, m_size };

        return
            chunk_pos.x >= min.x && chunk_pos.x < max.x &&
            chunk_pos.y >= min.y && chunk_pos.y < max.y;
    }

    v3i ChunkRegion::to_local_chunk_pos_3d(const v3i& chunk_pos) const
    {
        return chunk_pos - v3i{ m_corner_pos.x, 0, m_corner_pos.y };
    }

    v2i ChunkRegion::to_local_chunk_pos_2d(v2i chunk_pos) const
    {
        return chunk_pos - m_corner_pos;
    }

    constexpr size_t ChunkRegion::to_index(v2i pos) const
    {
        return pos.x * m_size + pos.y;
    }

    std::vector<i32> ChunkRegion::gen_new_to_old_indices(v2i new_corner_pos, u32 new_size) const
    {
        const auto in_bounds = [](v2i pos, u32 size) -> bool
        {
            return
                pos.x >= 0 && pos.x < size &&
                pos.y >= 0 && pos.y < size;
        };

        const v2i offset = new_corner_pos - m_corner_pos;

        std::vector<i32> new_indices(new_size * new_size, -1);
        for (i32 i = 0; i < new_size; i++)
        for (i32 j = 0; j < new_size; j++)
        {
            const v2i pos { i, j };
            const v2i old_pos = pos + offset;

            if (in_bounds(old_pos, m_size))
            {
                const i32 idx_new_array = i * i32(new_size) + j;
                const i32 idx_old_array = old_pos.x * i32(m_size) + old_pos.y;
                assert(idx_new_array >= 0 && idx_old_array >= 0);

                new_indices[idx_new_array] = idx_old_array;
            }
        }

        return new_indices;
    }
}