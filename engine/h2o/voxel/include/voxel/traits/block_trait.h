#pragma once

#include "voxel/block.h"

namespace h2o
{
    struct BlockModel;
    struct BlockType;

    class BlockTrait
    {
    public:

        BlockTrait() = delete;
        virtual ~BlockTrait() = default;

        [[nodiscard]] virtual u16 num_bits() const { return 0; }

        virtual void edit_block_model(Block block, BlockModel& block_model) const {}
        virtual void tick_block(Block& block) const {}

    protected:

        // Meant to be called by subclasses
        BlockTrait(const BlockType& block_type, u16 offset, u16 num_bits);

        [[nodiscard]] u16 read_data(Block block) const;

    protected:

        const BlockType* const m_block_type = nullptr;

    private:

        u16 m_data_mask = 0;
        u16 m_data_offset = 0;

    };
}