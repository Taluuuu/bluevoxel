#pragma once

#include "voxel_constants.h"

#include <array>
#include <memory>

namespace h2o
{
    class Chunk;
    using ChunkColumn = std::array<Chunk, voxel_constants::vertical_chunk_count>;
}