#include "voxel/block_type.h"

#include "voxel/traits/block_trait.h"

namespace h2o
{
    void BlockType::for_each_trait(const std::function<void(const BlockTrait&)>& function) const
    {
        for (const auto& trait : m_block_traits)
        {
            assert(trait != nullptr);
            function(*trait);
        }
    }

    u32 BlockType::calc_trait_bit_count() const
    {
        u32 bit_count = 0;
        for (const auto& trait : m_block_traits)
        {
            assert(trait != nullptr);
            bit_count += trait->num_bits();
        }

        return bit_count;
    }
}
