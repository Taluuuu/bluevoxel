#include "voxel/chunk_manager_base.h"

#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"

namespace h2o
{
    void ChunkManager_Base::fetch_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn*)>& function)
    {
        std::lock_guard lock { m_loaded_chunks_mutex };
        function(find_chunk_column(chunk_column_pos));
    }

    void ChunkManager_Base::fetch_chunk_column(v2i chunk_column_pos, const std::function<void(const ChunkColumn*)>& function) const
    {
        std::lock_guard lock { m_loaded_chunks_mutex };
        function(find_chunk_column(chunk_column_pos));
    }

    void ChunkManager_Base::fetch_or_create_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn&, bool)>& function)
    {
        std::lock_guard lock { m_loaded_chunks_mutex };

        bool was_just_created = false;
        auto& chunk_column = find_or_create_chunk_column(chunk_column_pos, was_just_created);

        function(chunk_column, was_just_created);
    }

    void ChunkManager_Base::fetch_chunk_region(v2i chunk_region_center, const std::function<void(const ChunkRegion&)>& function)
    {
        std::lock_guard lock { m_loaded_chunks_mutex };

        ChunkRegion chunk_region(chunk_region_center);

        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            const v2i chunk_column_pos = chunk_region_center + v2i{ i, j };
            chunk_region.add_chunk_column(find_or_create_chunk_column(chunk_column_pos));
        }

        function(chunk_region);
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

    ChunkColumn* ChunkManager_Base::find_chunk_column(v2i chunk_column_pos) const
    {
        if (auto it = m_loaded_chunks.find(chunk_column_pos); it != m_loaded_chunks.end())
            return it->second.get();

        return nullptr;
    }

    ChunkColumn& ChunkManager_Base::find_or_create_chunk_column(v2i chunk_column_pos)
    {
        bool _;
        return find_or_create_chunk_column(chunk_column_pos, _);
    }

    ChunkColumn& ChunkManager_Base::find_or_create_chunk_column(v2i chunk_column_pos, bool& out_was_just_created)
    {
        if (auto chunk_column = find_chunk_column(chunk_column_pos))
        {
            out_was_just_created = false;
            return *chunk_column;
        }

        auto [new_chunk_col_it, success] =
            m_loaded_chunks.insert({ chunk_column_pos, create_chunk_column(chunk_column_pos) });

        assert(success);
        assert(new_chunk_col_it->second);

        out_was_just_created = true;
        return *new_chunk_col_it->second;
    }
}