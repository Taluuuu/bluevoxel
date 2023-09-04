#include "voxel/chunk_region.h"

#include "voxel/chunk_column.h"
#include "voxel/chunk_system.h"
#include "voxel/voxel_utils.h"

#include <cassert>
#include <glm/gtx/norm.hpp>
#include <optional>
#include <set>

namespace h2o
{
    ChunkRegionOLD::ChunkRegionOLD(ChunkSystem& chunk_system)
        : m_chunk_system(&chunk_system)
    {
        chunk_system.on_chunk_column_loaded.add_listener(m_on_chunk_loaded_handle,
            [&](const ChunkColumnLoadEvent& event)
            {
                if (in_region_bounds(event.chunk_handle->chunk_column_pos()))
                    on_chunk_fetched(event.chunk_handle);
            }
        );
    }

    std::optional<Block> ChunkRegionOLD::get_block_at(const v3i& block_pos) const
    {
        const v3i chunk_pos = block_to_chunk_pos(block_pos);
        if (ChunkWeakHandle chunk = get_chunk_at(chunk_pos))
            return chunk->get_block_at(block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    std::optional<Block> ChunkRegionOLD::get_block_at(const v3i& block_pos, ChunkWeakHandle& out_chunk) const
    {
        const v3i chunk_pos = block_to_chunk_pos(block_pos);
        if (ChunkWeakHandle chunk = get_chunk_at(chunk_pos))
        {
            out_chunk = chunk;
            return chunk->get_block_at(block_pos_to_within_chunk(block_pos));
        }

        return std::nullopt;
    }

    bool ChunkRegionOLD::set_block_at(const v3i& block_pos, Block block) const
    {
        const v3i chunk_pos = block_to_chunk_pos(block_pos);
        if (ChunkWeakHandle chunk = get_chunk_at(chunk_pos))
        {
            chunk->set_block_at(block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    void ChunkRegionOLD::set_corner_pos(v2i new_corner_pos)
    {
        update_data(new_corner_pos, m_size);
    }

    void ChunkRegionOLD::set_size(u32 new_size)
    {
        update_data(m_corner_pos, new_size);
    }

    void ChunkRegionOLD::update_data(v2i new_corner_pos, u32 new_size)
    {
        assert(m_chunk_system);

        const auto new_to_old_indices = gen_new_to_old_indices(new_corner_pos, new_size);

        m_corner_pos = new_corner_pos;
        m_size = new_size;

        struct ChunkPosDistance
        {
            v2i local_pos{};
            f32 distance{};

            [[nodiscard]] bool operator<(const ChunkPosDistance& other) const
            { return distance < other.distance; }

            [[nodiscard]] bool operator==(const ChunkPosDistance& other) const
            { return local_pos == other.local_pos; }
        };

        std::multiset<ChunkPosDistance> chunk_positions_to_load;

        std::vector< WeakHandle<ChunkColumn> > new_chunks(new_size * new_size, nullptr);
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
                const v2i local_pos { i, j };
                chunk_positions_to_load.emplace(local_pos,
                    glm::distance2(v2(center_pos()), v2(corner_pos() + local_pos)));
            }
        }

        m_chunks_in_region = std::move(new_chunks);

        on_indices_changed(new_to_old_indices);

        for (const auto& local_chunk_pos : chunk_positions_to_load)
        {
            const v2i world_chunk_pos = local_chunk_pos.local_pos + new_corner_pos;

            const auto chunk_col = m_chunk_system->fetch_or_create_chunk_column(world_chunk_pos);

            m_chunks_in_region[to_index(local_chunk_pos.local_pos)] = chunk_col;
            if (chunk_col->is_generated())
            {
                on_chunk_fetched(chunk_col);
            }
            else
            {
                m_chunk_system->request_chunk_generation(chunk_col);
            }
        }
    }

    void ChunkRegionOLD::for_each_chunk(const std::function<void(Chunk&)>& fun) const
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

    ChunkWeakHandle ChunkRegionOLD::get_chunk_at(const v3i& chunk_pos) const
    {
        if (!in_region_bounds(chunk_pos))
            return nullptr;

        if (const auto chunk_col = get_chunk_col_at({ chunk_pos.x, chunk_pos.z }))
            return ChunkWeakHandle { chunk_col, chunk_pos.y }; // assumes chunk positions go from 0 to whatever;

        return nullptr;
    }

    WeakHandle<ChunkColumn> ChunkRegionOLD::get_chunk_col_at(v2i chunk_col_pos) const
    {
        if (const auto local_pos = to_local_chunk_pos_2d(chunk_col_pos))
        {
            const size_t index = to_index({ local_pos->x, local_pos->y });
            const auto& chunk_col = m_chunks_in_region[index];
            if (chunk_col)
                return chunk_col;
        }

        return nullptr;
    }

    bool ChunkRegionOLD::in_region_bounds(const v3i& chunk_pos) const
    {
        const v3i min { m_corner_pos.x, 0, m_corner_pos.y };
        const v3i max = min + v3i{ m_size, voxel_constants::vertical_chunk_count, m_size };

        return
            chunk_pos.x >= min.x && chunk_pos.x < max.x &&
            chunk_pos.y >= min.y && chunk_pos.y < max.y &&
            chunk_pos.z >= min.z && chunk_pos.z < max.z;
    }

    bool ChunkRegionOLD::in_region_bounds(v2i chunk_pos) const
    {
        const v2i min = m_corner_pos;
        const v2i max = min + v2i { m_size, m_size };

        return
            chunk_pos.x >= min.x && chunk_pos.x < max.x &&
            chunk_pos.y >= min.y && chunk_pos.y < max.y;
    }

    ChunkSystem& ChunkRegionOLD::chunk_system() const
    {
        assert(m_chunk_system);
        return *m_chunk_system;
    }

    std::optional<v3i> ChunkRegionOLD::to_local_chunk_pos_3d(const v3i& chunk_pos) const
    {
        if (!in_region_bounds(chunk_pos)) return std::nullopt;
        return chunk_pos - v3i{ m_corner_pos.x, 0, m_corner_pos.y };
    }

    std::optional<v2i> ChunkRegionOLD::to_local_chunk_pos_2d(v2i chunk_pos) const
    {
        if (!in_region_bounds(chunk_pos)) return std::nullopt;
        return chunk_pos - m_corner_pos;
    }

    std::vector<i32> ChunkRegionOLD::gen_new_to_old_indices(v2i new_corner_pos, u32 new_size) const
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

    StaticChunkRegion::StaticChunkRegion(v2i corner, i32 size, ChunkSystem& chunk_system)
        : m_corner(corner)
        , m_size(size)
    {
        m_chunks_in_region.resize(size * size, nullptr);

        for (i32 i = 0; i < size; i++ )
        for (i32 j = 0; j < size; j++ )
        {
            const v2i local_chunk_pos { i, j };
            const v2i chunk_col_pos = local_chunk_pos + corner;

            if (const auto local_chunk_col_pos = to_local_chunk_pos(chunk_col_pos))
            {
                const size_t index = to_index(*local_chunk_col_pos);
                m_chunks_in_region[index] = chunk_system.fetch_or_create_chunk_column(chunk_col_pos);
            }
        }
    }

    WeakHandle<ChunkColumn> StaticChunkRegion::get_center_chunk() const
    {
        const size_t index = to_index({ m_size / 2, m_size / 2 });
        assert(index < m_chunks_in_region.size());
        return m_chunks_in_region[index];
    }

    WeakHandle<ChunkColumn> StaticChunkRegion::get_chunk_col_at(v2i chunk_col_pos) const
    {
        if (const auto local_chunk_col_pos = to_local_chunk_pos(chunk_col_pos))
        {
            const size_t index = to_index(*local_chunk_col_pos);
            return m_chunks_in_region[index];
        }

        return nullptr;
    }

    void StaticChunkRegion::for_each_chunk_column(const std::function<void(const WeakHandle<ChunkColumn>&)>& fun) const
    {
        for (i32 i = 0; i < m_size; i++)
        for (i32 j = 0; j < m_size; j++)
        {
            const v2i chunk_col_pos = m_corner + v2i{ i, j };
            const auto chunk_col = get_chunk_col_at(chunk_col_pos);

            fun(chunk_col);
        }
    }

    bool StaticChunkRegion::is_in_region(v2i chunk_col_pos) const
    {
        const v2i min = m_corner;
        const v2i max = m_corner + v2i{ m_size, m_size };

        return
            chunk_col_pos.x >= min.x && chunk_col_pos.y >= min.y &&
            chunk_col_pos.x  < max.x && chunk_col_pos.y  < max.y;
    }

    std::optional<v2i> StaticChunkRegion::to_local_chunk_pos(v2i chunk_col_pos) const
    {
        if (!is_in_region(chunk_col_pos))
            return std::nullopt;

        return chunk_col_pos - m_corner;
    }

    size_t StaticChunkRegion::to_index(v2i local_chunk_pos) const
    {
        return local_chunk_pos.x * m_size + local_chunk_pos.y;
    }
}