#pragma once

#include "chunk.h"

namespace h2o
{
    // NOTE: These classes are not intended to be included in the module's public interface and
    //       thus should not be used by other modules (and should not need to be).
    //       They are here to prevent a circular include between ChunkManager and ChunkView.

    struct ChunkData
    {
        Chunk chunk{};
        std::shared_mutex mutex{};
    };

    using ChunkColumnData = std::array< ChunkData, voxel_constants::vertical_chunk_count >;
}
