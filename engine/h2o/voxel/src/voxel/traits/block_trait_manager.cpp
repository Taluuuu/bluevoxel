#include "voxel/traits/block_trait_manager.h"

#include "voxel/block_type.h"

#include <ranges>

namespace h2o
{
    bool BlockTraitManager::add_trait_to_block_type(const std::string& trait_name, BlockType& block_type) const
    {
        const auto trait_it = m_trait_builders.find(trait_name);
        if (trait_it == m_trait_builders.end())
            return false;

        if (auto trait = trait_it->second(block_type))
            return block_type.add_trait(trait);

        return false;
    }

    void BlockTraitManager::register_trait(const std::string& name, const BlockTraitBuilder& builder)
    {
        m_trait_builders[name] = builder;
    }

    std::vector<std::string> BlockTraitManager::trait_names() const
    {
        std::vector<std::string> result{};
        for (const auto& trait_name : std::views::keys(m_trait_builders))
            result.push_back(trait_name);

        return result;
    }
}
