#pragma once

#include "core/types.h"
#include "voxel/direction.h"

#include <array>
#include "magic_enum.hpp"
#include <vector>

namespace h2o
{
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

        [[nodiscard]] constexpr std::array<u32, 2> to_array() const
        {
            return
            {
                static_cast<u32>((x << 0) | (y << 10) | (z << 20)),
                static_cast<u32>((u << 0) | (v << 5) | (tex_idx << 10))
            };
        }
    };

    struct BlockModel
    {
        using Face = std::vector<BlockVertex>;
        std::array<std::vector<Face>, voxel::dir_count> occluded_vertices;
        std::vector<Face> unoccluded_vertices;
    };
}