#pragma once

#include "core/types.h"

namespace h2o::voxel_rendering_constants
{
    constexpr i32 num_normal_pitch_bits = 3;
    constexpr i32 packed_pitch_max_value = (1 << num_normal_pitch_bits) - 1;
    constexpr i32 num_normal_yaw_bits = 4;
    constexpr i32 packed_yaw_max_value = (1 << num_normal_yaw_bits) - 1;
}