#include "voxel/traits/block_trait.h"

namespace h2o
{
    u16 BlockTrait::read_data(const Block block) const
    {
        return block.data & m_data_mask >> m_data_offset;
    }

    BlockTrait::BlockTrait(const BlockType& block_type, const u16 offset, const u16 num_bits)
        : m_block_type(&block_type)
        , m_data_offset(offset)
    {
        m_data_mask = ((1 << num_bits) - 1) << offset;
    }
}
