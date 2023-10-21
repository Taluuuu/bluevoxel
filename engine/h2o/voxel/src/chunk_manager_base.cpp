#include "voxel/chunk_manager_base.h"

#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"

namespace h2o
{
    // Everything that adds, erases or finds from the chunk unordered_map is protected by the mutex.
    // Once a chunk is fetched, it is kept in a shared_ptr to prevent it from being deallocated while
    // the user runs a lambda. This shared_ptr is not meant to be exposed to the user as to make sure
    // interaction with chunks is only done in a safe environment.

    void ChunkManager_Base::fetch_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn*)>& function)
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
        {
            std::lock_guard chunk_column_lock { chunk_column->mutex() };
            function(chunk_column.get());

            return;
        }

        function(nullptr);
    }

    void ChunkManager_Base::fetch_chunk_column(v2i chunk_column_pos, const std::function<void(const ChunkColumn*)>& function) const
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
        {
            std::lock_guard chunk_column_lock { chunk_column->mutex() };
            function(chunk_column.get());

            return;
        }

        function(nullptr);
    }

    void ChunkManager_Base::fetch_or_create_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn&, bool)>& function)
    {
        bool was_just_created = false;
        auto chunk_column = find_or_create_chunk_column(chunk_column_pos, was_just_created);
        assert(chunk_column);

        std::lock_guard chunk_column_lock { chunk_column->mutex() };
        function(*chunk_column, was_just_created);
    }

    void ChunkManager_Base::fetch_chunk_region(v2i chunk_region_center, const std::function<void(const ChunkRegion&)>& function)
    {
        ChunkRegion chunk_region(chunk_region_center);

        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            const v2i chunk_column_pos = chunk_region_center + v2i{ i, j };
            auto chunk_column = find_or_create_chunk_column(chunk_column_pos);
            assert(chunk_column);

            chunk_column->mutex().lock(); // Ugly but simple
            chunk_region.add_chunk_column(chunk_column);
        }

        function(chunk_region);

        // Unlock mutexes
        chunk_region.for_each_chunk_column(
            [](ChunkColumn& chunk_column)
            {
                chunk_column.mutex().unlock();
            }
        );
    }

    void ChunkManager_Base::erase_far_chunks(const std::vector<v2i>& positions, i32 range)
    {
        std::lock_guard lock { m_loaded_chunks_mutex };

        erase_if(m_loaded_chunks,
            [&](const auto& item) -> bool
            {
                for (const v2i position : positions)
                {
                    const v2i min = position - v2i { range, range };
                    const v2i max = position + v2i { range, range };

                    if (position.x < min.x || position.x > max.x ||
                        position.y < min.y || position.y > max.y)
                    {
                        return true;
                    }
                }

                return false;
            }
        );
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::create_chunk_column(v2i chunk_column_pos) const
    {
        return std::make_shared<ChunkColumn>(chunk_column_pos);
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::find_chunk_column(v2i chunk_column_pos) const
    {
        std::lock_guard lock { m_loaded_chunks_mutex };
        if (auto it = m_loaded_chunks.find(chunk_column_pos); it != m_loaded_chunks.end())
            return it->second;

        return nullptr;
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::find_or_create_chunk_column(v2i chunk_column_pos)
    {
        bool _;
        return find_or_create_chunk_column(chunk_column_pos, _);
    }

    std::shared_ptr<ChunkColumn> ChunkManager_Base::find_or_create_chunk_column(v2i chunk_column_pos, bool& out_was_just_created)
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
        {
            out_was_just_created = false;
            return chunk_column;
        }

        std::lock_guard lock { m_loaded_chunks_mutex };
        auto [new_chunk_col_it, success] =
            m_loaded_chunks.insert({ chunk_column_pos, create_chunk_column(chunk_column_pos) });

        assert(success);
        assert(new_chunk_col_it->second);

        out_was_just_created = true;
        return new_chunk_col_it->second;
    }
}