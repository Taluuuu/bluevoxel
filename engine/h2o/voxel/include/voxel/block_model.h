#pragma once

#include "core/types.h"
#include "voxel/direction.h"
#include "voxel/voxel_constants.h"

#include <array>
#include <functional>
#include <magic_enum.hpp>
#include <string>
#include <vector>

namespace h2o
{
    constexpr u32 max_model_position = 16;

    struct BlockVertex
    {
        // Byte 1
        u32 x : 10;
        u32 y : 10;
        u32 z : 10;
        u32 : 0; // 30-bit

        // Byte 2
        u32 u : 5;
        u32 v : 5;
        u32 tex_idx : 11;
        u32 : 0; // 27-bit

        // Byte 3
        u32 n_pitch : voxel_constants::num_normal_pitch_bits;
        u32 n_yaw : voxel_constants::num_normal_yaw_bits;
        u32 light_level : voxel_constants::num_light_level_bits;
        u32 : 0; // 11-bit

        [[nodiscard]] constexpr std::array<u32, 3> to_array() const
        {
            return
            {
                static_cast<u32>((x << 0) | (y << 10) | (z << 20)),
                static_cast<u32>((u << 0) | (v << 5) | (tex_idx << 10)),
                static_cast<u32>((n_pitch << 0) | (n_yaw << 4) | (light_level << 9))
            };
        }
    };

    struct BlockModel
    {
        // TODO: Remove this...
        std::string name{};

        using Triangle = std::array<BlockVertex, 3>;

        std::array<std::vector<Triangle>, 6> occluded_triangles_per_side{};
        std::vector<Triangle> unoccluded_triangles{};

    };
}