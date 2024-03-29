#pragma once

#include "voxel/block.h"

#include <memory>
#include <vector>

namespace h2o::voxel
{
    struct BlockPlaceEvent
    {
        u32 client_id{}; // The client who set the block
        v3i block_pos{};
        Block block{};
    };

    struct ChunkColumnRequestedEvent
    {
        u32 client_id{}; // The client requesting the chunk columns
        std::vector<v2i> chunk_column_positions{};
    };

    struct ReceivedChunkColumnEvent
    {
        std::vector< std::shared_ptr<ChunkColumn> > chunk_column{};
    };
}