#pragma once

#include "chunk.h"
#include "chunk_lighting.h"

namespace h2o
{
    struct ChunkData
    {
        Chunk chunk{};
        std::shared_mutex chunk_mutex{};

        ChunkLighting chunk_lighting{};
        std::shared_mutex lighting_mutex{};
    };

    using ChunkColumnData = std::array< ChunkData, voxel_constants::vertical_chunk_count >;
}
