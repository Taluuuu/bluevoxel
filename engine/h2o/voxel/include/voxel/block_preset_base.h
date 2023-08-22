#pragma once

#include "block.h"

namespace h2o
{
    class Chunk;

    struct BlockPresetFlags
    {
        u8 should_tick : 1 = 0;
    };

    class BlockPreset_Base
    {
    public:

        [[nodiscard]] virtual bool should_tick(Block block) const { return false; }
        [[nodiscard]] virtual BlockPresetFlags preset_flags() const { return BlockPresetFlags{}; }
        virtual void tick(Block& block, Chunk& chunk, const v3i& local_block_pos) const;

    };
}