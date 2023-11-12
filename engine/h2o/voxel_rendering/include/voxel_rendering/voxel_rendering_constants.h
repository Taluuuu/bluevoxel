#pragma once

#include "core/types.h"

namespace h2o::voxel_rendering_constants
{
    // Pretty sure these values are 1-bit inefficient, but they will work for now.
    // Might revisit later.
    constexpr i32 num_normal_pitch_bits = 4;
    constexpr i32 packed_pitch_max_value = 8;//(1 << num_normal_pitch_bits) - 1;
    constexpr i32 num_normal_yaw_bits = 5;
    constexpr i32 packed_yaw_max_value = 16;//(1 << num_normal_yaw_bits) - 1;
}