#include "voxel/traits/block_trait.h"

#include <utility>

#include "voxel/block_type.h"

namespace h2o
{
    u16 BlockTrait::read_data(const Block block) const
    {
        return block.data & m_data_mask >> m_data_offset;
    }

    void BlockTrait::set_data(Block& block, u16 data) const
    {
        block.data = block.data & ~m_data_mask; // clear block data
        data = (data & (m_data_mask >> m_data_offset)); // make sure data is the right number of bits
        block.data = block.data | (data << m_data_offset); // set block data
    }

    BlockTrait::BlockTrait(std::string trait_name, const BlockType& block_type, const u16 num_bits)
        : m_block_type(&block_type)
        , m_data_offset(block_type.calc_trait_bit_count())
        , m_trait_name(std::move(trait_name))
    {
        m_data_mask = ((1 << num_bits) - 1) << m_data_offset;
    }
}
