#pragma once

#include "core/types.h"

namespace h2o::voxel_constants
{
    constexpr i32 chunk_size = 32;
    constexpr i32 chunk_area = chunk_size * chunk_size;
    constexpr i32 chunk_volume = chunk_area * chunk_size;
    constexpr i32 vertical_chunk_count = 8;
}