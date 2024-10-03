#include "voxel/traits/block_trait_rotation.h"

#include "voxel/block_model.h"
#include "voxel/voxel_utils.h"

#include <core/utils.h>

namespace h2o
{
    namespace int_math
    {
        static constexpr i32 cos(i32 half_pi_coef)
        {
            half_pi_coef = utils::non_stupid_mod(half_pi_coef, 4);
            switch (half_pi_coef)
            {
            case 0:  return  1;
            case 1:  return  0;
            case 2:  return -1;
            case 3:  return  0;
            default: return  1; // should not happen
            }
        }

        static constexpr i32 sin(i32 half_pi_coef)
        {
            half_pi_coef = utils::non_stupid_mod(half_pi_coef, 4);
            switch (half_pi_coef)
            {
            case 0:  return  0;
            case 1:  return  1;
            case 2:  return  0;
            case 3:  return -1;
            default: return  0; // should not happen
            }
        }

        static constexpr i32 atan2(const i32 x, const i32 y)
        {
            if (glm::abs(x) + glm::abs(y) != 1)
                return 0;

            if (x == 0)
                return (y == 1) ? 1 : 3;

            return (x == 1) ? 0 : 2;
        }

        static constexpr u32 index_of_y_rotated_direction(const u8 dir_index, const i32 half_pi_coeff)
        {
            // This function is really ugly, as I have to move the direction index into a workable space
            // to allow rotation.
            // This would not be necessary if directions were encoded in a rotatable order.

            constexpr std::array rotations {
                voxel::Direction::XNeg,
                voxel::Direction::ZNeg,
                voxel::Direction::XPos,
                voxel::Direction::ZPos,
            };

            constexpr std::array rotation_indices {
                0, 2, 1, 3
            };

            switch (voxel::to_direction(dir_index))
            {
            case voxel::Direction::YNeg: return voxel::to_index(voxel::Direction::YNeg);
            case voxel::Direction::YPos: return voxel::to_index(voxel::Direction::YPos);
            default:
                {
                    const auto local_rot_index = rotation_indices[dir_index];
                    const auto rot = rotations[utils::non_stupid_mod(local_rot_index - half_pi_coeff, 4)];
                    return voxel::to_index(rot);
                }
            }
        }
    }

    BlockTrait_Rotation::BlockTrait_Rotation(const BlockType& block_type)
        : BlockTrait(block_type, BlockTrait_Rotation::num_bits())
    {}

    Block BlockTrait_Rotation::rotate_to_normal(Block block, const v3i& normal) const
    {
        set_data(block, int_math::atan2(normal.x, normal.z));
        return block;
    }

    void BlockTrait_Rotation::edit_block_model(const Block block, BlockModel& block_model) const
    {
        BlockModel block_model_copy = block_model;

        const u16 rotation = read_data(block);

        const i32 sin_rot = int_math::sin(rotation);
        const i32 cos_rot = int_math::cos(rotation);

        const m3i rot_mat {
            cos_rot, 0, sin_rot,
            0, 1, 0,
            -sin_rot, 0, cos_rot
        };

        const auto rotate_vertex =
            [&](BlockVertex& vertex)
            {
                const v3i vertex_pos {
                    static_cast<i32>(vertex.x),
                    static_cast<i32>(vertex.y),
                    static_cast<i32>(vertex.z)
                };

                // Rotate the vertex pos around v3i{ max_model_position / 2 }
                const v3i centered_vertex_pos = vertex_pos - v3i{ max_model_position / 2 };
                const v3i rotated_vertex_pos = rot_mat * centered_vertex_pos;
                const v3i final_vertex_pos = rotated_vertex_pos + v3i{ max_model_position / 2 };

                vertex.x = final_vertex_pos.x;
                vertex.y = final_vertex_pos.y;
                vertex.z = final_vertex_pos.z;

                // Rotate the vertex's yaw by 90 degrees * rotation
                vertex.n_yaw = utils::non_stupid_mod(
                    vertex.n_yaw - rotation * voxel_constants::packed_yaw_max_value / 4,
                    voxel_constants::packed_yaw_max_value + 1
                );
            };

        for (auto& side : block_model_copy.occluded_triangles_per_side)
        {
            for (auto& triangle : side)
            {
                for (auto& vertex : triangle)
                    rotate_vertex(vertex);
            }
        }

        for (auto& triangle : block_model.unoccluded_triangles)
        {
            for (auto& vertex : triangle)
                rotate_vertex(vertex);
        }

        // Rotate occluded sides
        for (i32 i = 0; i < 6; i++)
        {
            const u32 rotated_index = int_math::index_of_y_rotated_direction(i, rotation);
            block_model.occluded_triangles_per_side[i] = std::move(
                block_model_copy.occluded_triangles_per_side[rotated_index]);
        }
    }
}
