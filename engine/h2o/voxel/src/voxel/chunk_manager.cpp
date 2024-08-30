#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "voxel/chunk_view.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    std::optional<Block> ChunkManager::get_block_at(const v3i& block_pos) const
    {
        std::optional<Block> block = std::nullopt;
        fetch_chunk(voxel_utils::block_to_chunk_pos(block_pos),
            [&](const Chunk* chunk)
            {
                if (chunk)
                    block = chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
            }
        );

        return block;
    }

    bool ChunkManager::set_block_at(const v3i& block_pos, Block block)
    {
        bool success = false;
        fetch_chunk(voxel_utils::block_to_chunk_pos(block_pos),
            [&](Chunk* chunk)
            {
                if (chunk)
                {
                    chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
                    success = true;
                }
            }
        );

        return success;
    }

    void ChunkManager::fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    {
        if (const auto chunk_col = find_chunk_column({ chunk_pos.x, chunk_pos.z }))
        {
            auto& [chunk, mutex] = (*chunk_col)[chunk_pos.y];
            const std::unique_lock lock { mutex };

            function(&chunk);
            return;
        }

        function(nullptr);
    }

    void ChunkManager::fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const
    {
        if (const auto chunk_col = find_chunk_column({ chunk_pos.x, chunk_pos.z }))
        {
            auto& [chunk, mutex] = (*chunk_col)[chunk_pos.y];
            const std::shared_lock lock { mutex };

            function(&chunk);
            return;
        }

        function(nullptr);
    }

    void ChunkManager::view_or_create_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(ChunkColumnView&)>& function)
    {
        {
            // Create it here if it does not already exist
            const auto chunk_column = find_or_create_chunk_column(chunk_column_pos);
            assert(chunk_column);
        }

        view({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(ChunkColumnView&)>& function)
    {
        view({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(const ChunkColumnView&)>& function) const
    {
        view({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    // void ChunkManager_Base::fetch_chunk_region(
    //     const std::vector<v3i>& chunk_positions,
    //     const std::function<void(const ChunkRegion_OLD&)>& function)
    // {
    //     assert(!chunk_positions.empty());
    //     v3i min { INT32_MAX, INT32_MAX, INT32_MAX };
    //     v3i max { INT32_MIN, INT32_MIN, INT32_MIN };
    //
    //     // Find min and max chunk positions.
    //     // NOTE: This might be slow for large amounts of chunks.
    //     for (const v3i& chunk_pos : chunk_positions)
    //     {
    //         min.x = chunk_pos.x < min.x ? chunk_pos.x : min.x;
    //         min.y = chunk_pos.y < min.y ? chunk_pos.y : min.y;
    //         min.z = chunk_pos.z < min.z ? chunk_pos.z : min.z;
    //
    //         max.x = chunk_pos.x > max.x ? chunk_pos.x : max.x;
    //         max.y = chunk_pos.y > max.y ? chunk_pos.y : max.y;
    //         max.z = chunk_pos.z > max.z ? chunk_pos.z : max.z;
    //     }
    //
    //     ChunkRegion_OLD chunk_region(min, max - min + v3i{ 1, 1, 1 });
    //
    //     // Make sure chunk columns don't get deallocated.
    //     std::vector< std::shared_ptr<ChunkColumn> > chunk_columns;
    //
    //     for (const v3i& chunk_pos : chunk_positions)
    //     {
    //         if (auto chunk_column = find_chunk_column({ chunk_pos.x, chunk_pos.z }))
    //         {
    //             chunk_columns.push_back(chunk_column);
    //
    //             if (Chunk* chunk = chunk_column->get_chunk_safe(chunk_pos.y))
    //                 chunk_region.add_chunk(*chunk);
    //         }
    //     }
    //
    //     // Non-exclusive lock, as the chunk region is const.
    //     chunk_region.lock_chunks(false);
    //     function(chunk_region);
    //     chunk_region.unlock_chunks(false);
    // }

    // void ChunkManager_Base::fetch_chunk_region(
    //     v3i min, v3i max,
    //     const std::function<void(ChunkRegion_OLD&)>& function)
    // {
    //     assert(min.x <= max.x && min.y <= max.y && min.z <= max.z);
    //
    //     ChunkRegion_OLD chunk_region(min, max - min + v3i{ 1, 1, 1 });
    //
    //     // Make sure chunk columns don't get deallocated.
    //     std::vector< std::shared_ptr<ChunkColumn> > chunk_columns {
    //         size_t((max.x - min.x + 1) * (max.z - min.z + 1)), nullptr };
    //
    //     for (i32 i = min.x; i <= max.x; i++)
    //     for (i32 j = min.y; j <= max.y; j++)
    //     {
    //         const v2i chunk_column_pos { i, j };
    //         if (auto chunk_column = find_chunk_column(chunk_column_pos))
    //         {
    //             chunk_columns.push_back(chunk_column);
    //
    //             for (i32 y = min.y; y <= max.y; y++)
    //             {
    //                 if (Chunk* chunk = chunk_column->get_chunk_safe(y))
    //                     chunk_region.add_chunk(*chunk);
    //             }
    //         }
    //     }
    //
    //     // Exclusive lock, as the chunk region is non-const.
    //     chunk_region.lock_chunks(true);
    //     function(chunk_region);
    //     chunk_region.unlock_chunks(true);
    // }

    // void ChunkManager_Base::erase_far_chunks(const std::vector<v2i>& positions, i32 range)
    // {
    //     std::unique_lock lock { m_loaded_chunks_mutex };
    //
    //     const size_t num_erased_chunks = erase_if(m_loaded_chunks,
    //         [&](const auto& item) -> bool
    //         {
    //             for (const v2i position : positions)
    //             {
    //                 const v2i min = position - v2i { range, range };
    //                 const v2i max = position + v2i { range, range };
    //
    //                 if (item.first.x < min.x || item.first.x > max.x ||
    //                     item.first.y < min.y || item.first.y > max.y)
    //                 {
    //                     return true;
    //                 }
    //             }
    //
    //             return false;
    //         }
    //     );
    //
    //     log::info("Erased {} chunks.", num_erased_chunks);
    // }

    std::shared_ptr<ChunkManager::ChunkColumnData> ChunkManager::create_chunk_column(v2i chunk_column_pos)
    {
        const auto chunk_col = std::make_shared<ChunkColumnData>();
        for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
        {
            (*chunk_col)[i].chunk.init(
                { chunk_column_pos.x, i, chunk_column_pos.y },
                g_engine->get_module_checked<VoxelModule>());
        }

        return chunk_col;
    }

    std::shared_ptr<ChunkManager::ChunkColumnData> ChunkManager::find_chunk_column(v2i chunk_column_pos) const
    {
        const std::shared_lock lock { m_loaded_chunks_mutex };
        if (const auto it = m_loaded_chunks.find(chunk_column_pos); it != m_loaded_chunks.end())
            return it->second;

        return nullptr;
    }

    std::shared_ptr<ChunkManager::ChunkColumnData> ChunkManager::find_or_create_chunk_column(v2i chunk_column_pos)
    {
        if (const auto chunk_column = find_chunk_column(chunk_column_pos))
            return chunk_column;

        const std::unique_lock lock { m_loaded_chunks_mutex };

        const auto [it, success] = m_loaded_chunks.try_emplace(chunk_column_pos, nullptr);
        if (success)
            it->second = create_chunk_column(chunk_column_pos);

        return it->second;
    }
}