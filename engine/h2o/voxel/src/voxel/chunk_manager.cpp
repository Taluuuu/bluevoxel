#include "voxel/chunk_manager_interface.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    // void IChunkManager_OLD::fetch_chunk_region(v2i min, v2i max, const std::function<void(ChunkRegion_OLD&)>& function)
    // {
    //     fetch_chunk_region(
    //         v3i{ min.x, 0, min.y },
    //         v3i{ max.x, voxel_constants::vertical_chunk_count - 1, max.y }, function);
    // }
    //
    // void IChunkManager_OLD::fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    // {
    //     fetch_chunk_column({ chunk_pos.x, chunk_pos.z }, false,
    //         [&](ChunkColumn* chunk_column)
    //         {
    //             if (!chunk_column)
    //             {
    //                 function(nullptr);
    //                 return;
    //             }
    //
    //             // if (Chunk* chunk = chunk_column->get_chunk_safe(chunk_pos.y))
    //             // {
    //             //     std::unique_lock lock { chunk->mutex() };
    //             //     function(chunk);
    //             // }
    //             // else
    //             // {
    //             //     function(nullptr);
    //             // }
    //         }
    //     );
    // }
    //
    // void IChunkManager_OLD::fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const
    // {
    //     fetch_chunk_column({ chunk_pos.x, chunk_pos.z }, false,
    //         [&](const ChunkColumn* chunk_column)
    //         {
    //             if (!chunk_column)
    //             {
    //                 function(nullptr);
    //                 return;
    //             }
    //
    //             // if (const Chunk* chunk = chunk_column->get_chunk_safe(chunk_pos.y))
    //             // {
    //             //     std::shared_lock lock { chunk->mutex() };
    //             //     function(chunk);
    //             // }
    //             // else
    //             // {
    //             //     function(nullptr);
    //             // }
    //         }
    //     );
    // }
}