#include "voxel/uncooked_block_model.h"

#include "voxel/voxel_constants.h"
#include "voxel/voxel_pack.h"

namespace h2o
{
    UncookedBlockModel::UncookedBlockModel(const std::string& name, u32 id)
        : m_name(name), m_id(id)
    {}

    static BlockModel::Triangle process_triangle(const UncookedBlockModel::Triangle& unprocessed_triangle, u32 face_index)
    {
        BlockModel::Triangle triangle{};

        const auto& v0 = unprocessed_triangle[0];
        const auto& v1 = unprocessed_triangle[1];
        const auto& v2 = unprocessed_triangle[2];

        const v3i p0{ v0[0], v0[1], v0[2] };
        const v3i p1{ v1[0], v1[1], v1[2] };
        const v3i p2{ v2[0], v2[1], v2[2] };

        // Pack normal vector
        const v3 p0_to_p1 = p1 - p0;
        const v3 p0_to_p2 = p2 - p0;
        const v3 normal = glm::normalize(glm::cross(p0_to_p1, p0_to_p2));
        const f32 n_pitch = std::asin(normal.y);
        const f32 n_yaw = std::atan2(normal.x, normal.z);

        // Remap from 0 to 1 to integer values depending on their number of bits
        const f32 normalized_pitch = (n_pitch + glm::half_pi<f32>()) / glm::pi<f32>();
        const f32 normalized_yaw = (n_yaw + glm::pi<f32>()) / glm::two_pi<f32>();

        const u32 packed_n_pitch = std::lround(normalized_pitch * voxel_constants::packed_pitch_max_value);
        const u32 packed_n_yaw = std::lround(normalized_yaw * voxel_constants::packed_yaw_max_value);

        for (i32 vertex_index = 0; vertex_index < 3; vertex_index++)
        {
            const auto& vertex = unprocessed_triangle[vertex_index];
            triangle[vertex_index] =
                BlockVertex
                {
                    .x = vertex[0],
                    .y = vertex[1],
                    .z = vertex[2],
                    .u = vertex[3],
                    .v = vertex[4],
                    .tex_idx = face_index,
                    .n_pitch = packed_n_pitch,
                    .n_yaw = packed_n_yaw,
                };
        }

        return triangle;
    }

    h2o::BlockModel UncookedBlockModel::build() const
    {
        h2o::BlockModel result{};

        u32 face_index = 0;
        for (const auto& face : m_faces)
        {
            for (const auto& triangle : face)
            {
                using namespace h2o;

                voxel::Direction::Type triangle_occluding_directions = voxel::Direction::All;

                // Find which faces need to be occluded by which axis
                i32 axis_index = 0;
                magic_enum::enum_for_each<voxel::Axis>([&](voxel::Axis axis)
                {
                    const auto dir = voxel::to_direction(axis);
                    const auto inv_dir = voxel::invert(dir);

                    for (const auto& vertex : triangle)
                    {
                        if (vertex[axis_index] > 0)
                        {
                            // Triangle can't be occluded by neighboring block at inv_dir
                            triangle_occluding_directions = static_cast<voxel::Direction::Type>(
                                triangle_occluding_directions & ~inv_dir);
                        }

                        if (vertex[axis_index] < voxel_constants::max_coord_value_per_block)
                        {
                            // Triangle can't be occluded by neighboring block at dir
                            triangle_occluding_directions = static_cast<voxel::Direction::Type>(
                                triangle_occluding_directions & ~dir);
                        }
                    }

                    axis_index++;
                });

                if (triangle_occluding_directions)
                {
                    for (i32 i = 0; i < 6; i++)
                    {
                        if (triangle_occluding_directions & (1 << i))
                        {
                            const auto processed_triangle = process_triangle(triangle, face_index);
                            result.occluded_triangles_per_side[i].push_back(processed_triangle);
                            break;
                        }
                    }
                }
                else
                {
                    // The triangle does not need to be occluded
                    const auto processed_triangle = process_triangle(triangle, face_index);
                    result.unoccluded_triangles.push_back(processed_triangle);
                }
            }

            face_index++;
        }

        return result;
    }

    void UncookedBlockModel::add_face(const UncookedBlockModel::Face& face)
    {
        m_faces.push_back(face);
    }
}