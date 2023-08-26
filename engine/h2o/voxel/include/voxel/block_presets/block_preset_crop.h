#pragma once

#include "block_preset_base.h"

namespace h2o
{
    class BlockPreset_Crop : public BlockPreset_Base
    {
    public:

        [[nodiscard]] BlockPresetFlags preset_flags() const override
        { return BlockPresetFlags{ .should_tick = true }; }

        void tick(Block& block, Chunk& chunk, const v3i& local_block_pos) const override;

    };
}