#pragma once

#include "block_container_interface.h"
#include "core/handle_types.h"

namespace h2o
{
    class Chunk;

    struct VoxelFetchResult
    {
        Chunk* chunk = nullptr;
        v3i pos { 0, 0, 0 };
        Block block = Block::Air;
    };

    struct VoxelRayHit
    {
        VoxelFetchResult hit_voxel;
        VoxelFetchResult before_hit_voxel;

        void add_voxel(const VoxelFetchResult& voxel_fetch_result)
        {
            before_hit_voxel = hit_voxel;
            hit_voxel = voxel_fetch_result;
        }
    };

    class VoxelRay
    {
    public:

        VoxelRay(
            const v3& origin,
            const v3& end,
            IBlockContainer& block_container);

        [[nodiscard]] operator bool() const { return has_hit(); }
        [[nodiscard]] bool has_hit()  const { return m_has_hit; }

        [[nodiscard]] const VoxelRayHit& hit() const;

    private:

        VoxelRayHit m_ray_hit;

        bool m_has_hit = false;

    };
}