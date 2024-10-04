#include "voxel/block_type.h"

#include "voxel/traits/block_trait.h"

namespace h2o
{
    bool BlockType::add_trait(std::shared_ptr<BlockTrait>& trait)
    {
        if (!trait)
            return false;

        const u32 current_bit_count = calc_trait_bit_count();
        if (current_bit_count + trait->num_bits() > max_data_bits)
        {
            log::warn("Trying to add too many data bits to block type: {}", name);
            return false;
        }

        m_block_traits.emplace_back(std::move(trait));
        return true;
    }

    void BlockType::remove_trait(const std::string& name)
    {
        erase_if(m_block_traits,
            [&](const std::shared_ptr<BlockTrait>& trait)
            {
                assert(trait != nullptr);
                return trait->trait_name() == name;
            }
        );
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

    const BlockTrait* BlockType::find_trait(const std::function<bool(const BlockTrait&)>& predicate) const
    {
        for (const auto& trait : m_block_traits)
        {
            assert(trait != nullptr);
            if (predicate(*trait))
                return trait.get();
        }

        return nullptr;
    }

    bool BlockType::has_trait(const std::string& trait_name) const
    {
        return nullptr != find_trait(
            [&](const BlockTrait& trait) { return trait.trait_name() == trait_name; });
    }

    void BlockType::for_each_trait(const std::function<void(const BlockTrait&)>& function) const
    {
        for (const auto& trait : m_block_traits)
        {
            assert(trait != nullptr);
            function(*trait);
        }
    }
}
