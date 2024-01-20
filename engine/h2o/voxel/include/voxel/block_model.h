#pragma once

#include "core/types.h"
#include "voxel/direction.h"
#include "voxel/voxel_constants.h"

#include <array>
#include <magic_enum.hpp>
#include <string>
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

        // Byte 3
        u32 n_pitch : voxel_constants::num_normal_pitch_bits;
        u32 n_yaw : voxel_constants::num_normal_yaw_bits;
        u32 : 0; // 7-bit

        [[nodiscard]] constexpr std::array<u32, 3> to_array() const
        {
            return
            {
                static_cast<u32>((x << 0) | (y << 10) | (z << 20)),
                static_cast<u32>((u << 0) | (v << 5) | (tex_idx << 10)),
                static_cast<u32>((n_pitch << 0) | (n_yaw << 4))
            };
        }
    };

    struct BlockModel
    {
        std::string name{};
        u32 id = 0;

        using Triangle = std::array<BlockVertex, 3>;
        using Face = std::vector<Triangle>;

        std::array<std::vector<Face>, 6> occluded_faces_per_side{};
        std::vector<Face> unoccluded_faces{};

        [[nodiscard]] u32 calculate_face_count() const
        {
            u32 face_count = unoccluded_faces.size();
            for (const auto& side : occluded_faces_per_side)
                face_count += side.size();

            return face_count;
        }
    };
}