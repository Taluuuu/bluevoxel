#pragma once

#include "core/types.h"

namespace h2o::voxel_constants
{
    constexpr i32 chunk_size = 32;
    constexpr i32 chunk_area = chunk_size * chunk_size;
    constexpr i32 chunk_volume = chunk_area * chunk_size;

    constexpr i32 vertical_chunk_count = 8;
    constexpr i32 vertical_block_count = vertical_chunk_count * chunk_size;

    constexpr i32 max_generation_stage = 4096;

    constexpr i32 max_coord_value_per_block = 16;
    constexpr i32 max_uv_value_per_block = 16;

    // Pretty sure these values are 1-bit inefficient, but they will work for now.
    // Might revisit later.
    constexpr i32 num_normal_pitch_bits = 4;
    constexpr i32 packed_pitch_max_value = 8;//(1 << num_normal_pitch_bits) - 1;
    constexpr i32 num_normal_yaw_bits = 5;
    constexpr i32 packed_yaw_max_value = 16;//(1 << num_normal_yaw_bits) - 1;
}