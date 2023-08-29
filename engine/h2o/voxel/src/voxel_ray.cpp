#include "voxel/voxel_ray.h"

#include "voxel/voxel_utils.h"

namespace h2o
{
    VoxelRay::VoxelRay(const v3& origin, const v3& end, IBlockContainer& block_container)
    {
        const v3 to_end = end - origin;
        const v3 direction = glm::normalize(to_end);
        const f32 range = glm::length(to_end);

        v3i current_voxel = world_to_block_pos(origin);

        const v3i step {
            direction.x > 0.0f ? 1 : ((direction.x < 0.0f) ? -1 : 0),
            direction.y > 0.0f ? 1 : ((direction.y < 0.0f) ? -1 : 0),
            direction.z > 0.0f ? 1 : ((direction.z < 0.0f) ? -1 : 0),
        };

        const v3 t_delta {
            (step.x != 0) ? glm::min(f32(step.x) / to_end.x, FLT_MAX) : FLT_MAX,
            (step.y != 0) ? glm::min(f32(step.y) / to_end.y, FLT_MAX) : FLT_MAX,
            (step.z != 0) ? glm::min(f32(step.z) / to_end.z, FLT_MAX) : FLT_MAX,
        };

        v3 t_max {
            (step.x > 0) ? t_delta.x * (1.0f - origin.x + f32(current_voxel.x)) : t_delta.x * (origin.x - f32(current_voxel.x)),
            (step.y > 0) ? t_delta.y * (1.0f - origin.y + f32(current_voxel.y)) : t_delta.y * (origin.y - f32(current_voxel.y)),
            (step.z > 0) ? t_delta.z * (1.0f - origin.z + f32(current_voxel.z)) : t_delta.z * (origin.z - f32(current_voxel.z)),
        };

        i32 intersection_count = 0;
        const i32 max_intersections = 3 * i32(glm::ceil(range));
        while (intersection_count < max_intersections)
        {
            intersection_count++;

            ChunkWeakHandle chunk;
            const auto block = block_container.get_block_at(current_voxel, chunk);

            if (!chunk || !block) // Out of bounds ?
                break;

            m_ray_hit.add_voxel({ chunk, current_voxel, *block });
            if (block != Block::Air)
            {
                m_has_hit = true;
                break;
            }

            if (t_max.x < t_max.y)
            {
                if (t_max.x < t_max.z)
                {
                    current_voxel.x += step.x;
                    t_max.x += t_delta.x;
                }
                else
                {
                    current_voxel.z += step.z;
                    t_max.z += t_delta.z;
                }
            }
            else
            {
                if (t_max.y < t_max.z)
                {
                    current_voxel.y += step.y;
                    t_max.y += t_delta.y;
                }
                else
                {
                    current_voxel.z += step.z;
                    t_max.z += t_delta.z;
                }
            }

            if (t_max.x > 1.0f && t_max.y > 1.0f && t_max.z > 1.0f)
                break;
        }
    }

    const VoxelRayHit& VoxelRay::hit() const
    {
        assert(has_hit());
        return m_ray_hit;
    }
}