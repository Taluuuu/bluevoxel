#include "voxel/chunk_manager_base.h"

#include "core/log.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"

namespace h2o
{
    void ChunkManager_Base::fetch_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(ChunkColumn*)>& function)
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
        {
            std::lock_guard chunk_column_lock { chunk_column->mutex() };

            if (lock_chunks)
            {
                for (Chunk& chunk : (*chunk_column))
                    chunk.mutex().lock();
            }

            function(chunk_column.get());

            if (lock_chunks)
            {
                for (Chunk& chunk : (*chunk_column))
                    chunk.mutex().unlock();
            }

            return;
        }

        function(nullptr);
    }

    void ChunkManager_Base::fetch_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(const ChunkColumn*)>& function) const
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
        {
            std::lock_guard chunk_column_lock { chunk_column->mutex() };

            if (lock_chunks)
            {
                for (Chunk& chunk: (*chunk_column))
                    chunk.mutex().lock_shared();
            }

            function(chunk_column.get());

            if (lock_chunks)
            {
                for (Chunk& chunk: (*chunk_column))
                    chunk.mutex().unlock_shared();
            }

            return;
        }

        function(nullptr);
    }

    void ChunkManager_Base::fetch_or_create_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(ChunkColumn&)>& function)
    {
        bool was_just_created = false;
        auto chunk_column = find_or_create_chunk_column(chunk_column_pos);
        assert(chunk_column);

        std::lock_guard chunk_column_lock { chunk_column->mutex() };

        if (lock_chunks)
        {
            for (Chunk& chunk: (*chunk_column))
                chunk.mutex().lock();
        }

        function(*chunk_column);

        if (lock_chunks)
        {
            for (Chunk& chunk: (*chunk_column))
                chunk.mutex().unlock();
        }
    }

    void ChunkManager_Base::fetch_chunk_region(
        const std::vector<v3i>& chunk_positions,
        const std::function<void(const ChunkRegion_OLD&)>& function)
    {
        assert(!chunk_positions.empty());
        v3i min { INT32_MAX, INT32_MAX, INT32_MAX };
        v3i max { INT32_MIN, INT32_MIN, INT32_MIN };

        // Find min and max chunk positions.
        // NOTE: This might be slow for large amounts of chunks.
        for (const v3i& chunk_pos : chunk_positions)
        {
            min.x = chunk_pos.x < min.x ? chunk_pos.x : min.x;
            min.y = chunk_pos.y < min.y ? chunk_pos.y : min.y;
            min.z = chunk_pos.z < min.z ? chunk_pos.z : min.z;

            max.x = chunk_pos.x > max.x ? chunk_pos.x : max.x;
            max.y = chunk_pos.y > max.y ? chunk_pos.y : max.y;
            max.z = chunk_pos.z > max.z ? chunk_pos.z : max.z;
        }

        ChunkRegion_OLD chunk_region(min, max - min + v3i{ 1, 1, 1 });

        // Make sure chunk columns don't get deallocated.
        std::vector< std::shared_ptr<ChunkColumn> > chunk_columns;

        for (const v3i& chunk_pos : chunk_positions)
        {
            if (auto chunk_column = find_chunk_column({ chunk_pos.x, chunk_pos.z }))
            {
                chunk_columns.push_back(chunk_column);

                if (Chunk* chunk = chunk_column->get_chunk_safe(chunk_pos.y))
                    chunk_region.add_chunk(*chunk);
            }
        }

        // Non-exclusive lock, as the chunk region is const.
        chunk_region.lock_chunks(false);
        function(chunk_region);
        chunk_region.unlock_chunks(false);
    }

    void ChunkManager_Base::fetch_chunk_region(
        v3i min, v3i max,
        const std::function<void(ChunkRegion_OLD&)>& function)
    {
        assert(min.x <= max.x && min.y <= max.y && min.z <= max.z);

        ChunkRegion_OLD chunk_region(min, max - min + v3i{ 1, 1, 1 });

        // Make sure chunk columns don't get deallocated.
        std::vector< std::shared_ptr<ChunkColumn> > chunk_columns {
            size_t((max.x - min.x + 1) * (max.z - min.z + 1)), nullptr };

        for (i32 i = min.x; i <= max.x; i++)
        for (i32 j = min.y; j <= max.y; j++)
        {
            const v2i chunk_column_pos { i, j };
            if (auto chunk_column = find_chunk_column(chunk_column_pos))
            {
                chunk_columns.push_back(chunk_column);

                for (i32 y = min.y; y <= max.y; y++)
                {
                    if (Chunk* chunk = chunk_column->get_chunk_safe(y))
                        chunk_region.add_chunk(*chunk);
                }
            }
        }

        // Exclusive lock, as the chunk region is non-const.
        chunk_region.lock_chunks(true);
        function(chunk_region);
        chunk_region.unlock_chunks(true);
    }

    void ChunkManager_Base::erase_far_chunks(const std::vector<v2i>& positions, i32 range)
    {
        std::unique_lock lock { m_loaded_chunks_mutex };

        const size_t num_erased_chunks = erase_if(m_loaded_chunks,
            [&](const auto& item) -> bool
            {
                for (const v2i position : positions)
                {
                    const v2i min = position - v2i { range, range };
                    const v2i max = position + v2i { range, range };

                    if (item.first.x < min.x || item.first.x > max.x ||
                        item.first.y < min.y || item.first.y > max.y)
                    {
                        return true;
                    }
                }

                return false;
            }
        );

        log::info("Erased {} chunks.", num_erased_chunks);
    }

    bool ChunkManager_Base::is_chunk_column_generated(v2i chunk_column_pos) const
    {
        if (const auto chunk_column = find_chunk_column(chunk_column_pos))
            return chunk_column->is_generated();

        return false;
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::create_chunk_column(v2i chunk_column_pos) const
    {
        return std::make_shared<ChunkColumn>(chunk_column_pos);
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::find_chunk_column(v2i chunk_column_pos) const
    {
        std::shared_lock lock { m_loaded_chunks_mutex };
        if (auto it = m_loaded_chunks.find(chunk_column_pos); it != m_loaded_chunks.end())
            return it->second;

        return nullptr;
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::find_or_create_chunk_column(v2i chunk_column_pos)
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
            return chunk_column;

        std::unique_lock lock { m_loaded_chunks_mutex };

        auto [it, success] = m_loaded_chunks.try_emplace(chunk_column_pos, nullptr);
        if (success)
            it->second = create_chunk_column(chunk_column_pos);

        return it->second;
    }
}