#pragma once

#include "block_container_interface.h"
#include "core/handle_types.h"

namespace h2o
{
    class Chunk;

    struct VoxelRayHit
    {
        ChunkWeakHandle chunk { nullptr };
        ChunkWeakHandle chunk_before { nullptr };

        v3i pos { 0, 0, 0 };
        v3i pos_before { 0, 0, 0 };

        Block block { Block::Air };
        Block block_before { Block::Air };
    };

    class VoxelRay
    {
    public:

        VoxelRay(
            const v3& origin,
            const v3& end,
            IBlockContainer& block_container);

        [[nodiscard]] bool has_hit() const { return m_has_hit; }
        [[nodiscard]] const VoxelRayHit& hit() const;

        [[nodiscard]] operator bool() const;

    private:

        VoxelRayHit m_ray_hit;

        bool m_has_hit = false;

    };
}