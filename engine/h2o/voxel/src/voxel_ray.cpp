#include "voxel/voxel_ray.h"

#include "voxel/voxel_utils.h"

namespace h2o
{
    VoxelRay::VoxelRay(const v3& origin, const v3& end, IBlockContainer& block_container)
    {
        const v3 to_end = end - origin;
        const v3 direction = glm::normalize(to_end);
        const f32 range = glm::length(to_end);

        const v3i current_voxel = world_to_block_pos(origin);

        const v3i step {
            direction.x > 0.0f ? 1 : ((direction.x < 0.0f) ? -1 : 0),
            direction.y > 0.0f ? 1 : ((direction.y < 0.0f) ? -1 : 0),
            direction.z > 0.0f ? 1 : ((direction.z < 0.0f) ? -1 : 0),
        };

        const v3 delta_t {
            (step.x != 0) ? glm::min(f32(step.x) / to_end.x, FLT_MAX) : FLT_MAX,
            (step.y != 0) ? glm::min(f32(step.y) / to_end.y, FLT_MAX) : FLT_MAX,
            (step.z != 0) ? glm::min(f32(step.z) / to_end.z, FLT_MAX) : FLT_MAX,
        };

        const v3 time_max {
            (step.x > 0) ? delta_t.x * (1.0f - origin.x + f32(current_voxel.x)) : delta_t.x * (origin.x - f32(current_voxel.x)),
            (step.y > 0) ? delta_t.y * (1.0f - origin.y + f32(current_voxel.y)) : delta_t.y * (origin.y - f32(current_voxel.y)),
            (step.z > 0) ? delta_t.z * (1.0f - origin.z + f32(current_voxel.z)) : delta_t.z * (origin.z - f32(current_voxel.z)),
        };

        i32 intersection_count = 0;
        const i32 max_intersections = 3 * i32(glm::ceil(range));
        while (intersection_count < max_intersections)
        {
//            m_ray_hit.block = block_container.get_block_at(current_voxel, m_ray_hit.chunk);
//            if (m_ray_hit.block != Block::Air)
//            {
//                m_ray_hit.pos = current_voxel;
//            }

            intersection_count++;
        }
    }

    const VoxelRayHit& VoxelRay::hit() const
    {
        assert(has_hit());
        return m_ray_hit;
    }

    VoxelRay::operator bool() const
    {
        return has_hit();
    }
}