#pragma once

#include "block_trait.h"

namespace h2o
{
    class BlockTrait_Rotation final : public BlockTrait
    {
    public:

        BlockTrait_Rotation(const BlockType& block_type, u16 offset);

        // BlockTrait interface
        [[nodiscard]] u16 num_bits() const override { return 2; }
        void edit_block_model(Block block, BlockModel& block_model) const override;

    };
}