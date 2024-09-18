#pragma once

#include "voxel/block.h"

namespace h2o
{
    class ChunkManager;

    class IBlockPlaceable
    {
    public:

        virtual ~IBlockPlaceable() = default;

        virtual void set_block_at(const v3i& block_pos, Block block) = 0;

        [[nodiscard]] virtual ChunkManager& chunk_mgr() = 0;
        [[nodiscard]] virtual const ChunkManager& chunk_mgr() const = 0;

    };
}