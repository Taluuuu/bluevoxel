#pragma once

#include "voxel/block.h"

namespace h2o
{
    struct BlockModel;
    class BlockType;

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
        explicit BlockTrait(const BlockType& block_type, u16 num_bits);

        [[nodiscard]] u16 read_data(Block block) const;
        void set_data(Block& block, u16 data) const;

    protected:

        const BlockType* const m_block_type = nullptr;

    private:

        u16 m_data_mask = 0;
        u16 m_data_offset = 0;

    };
}