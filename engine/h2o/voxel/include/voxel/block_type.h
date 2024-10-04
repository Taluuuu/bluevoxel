#pragma once

#include "block.h"
#include "core/log.h"
#include "core/types.h"
#include "traits/block_trait.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace h2o
{
    class BlockType final
    {
    public:

        BlockType() = default;
        BlockType(
            std::string name,
            const BlockID block_id,
            std::vector<u32> texture_ids,
            const u32 model_id,
            const bool is_transparent)
            : name(std::move(name))
            , block_id(block_id)
            , texture_ids(std::move(texture_ids))
            , model_id(model_id)
            , is_transparent(is_transparent) {}

        // Returns true on success
        bool add_trait(std::shared_ptr<BlockTrait>& trait);
        void remove_trait(const std::string& name);

        template<class T> [[nodiscard]] const T* query_trait() const;
        template<class T> [[nodiscard]] T* query_trait();

        // Calculate the number of bits in the data field currently in use
        [[nodiscard]] u32 calc_trait_bit_count() const;
        // Returns null if none match the predicate
        [[nodiscard]] const BlockTrait* find_trait(const std::function<bool(const BlockTrait&)>& predicate) const;
        [[nodiscard]] bool has_trait(const std::string& trait_name) const;
        void for_each_trait(const std::function<void(const BlockTrait&)>& function) const;

    public:

        std::string name{};
        BlockID block_id = 0;

        std::vector<u32> texture_ids{};
        u32 model_id = 0;

        bool is_transparent = false;

    private:

        std::vector< std::shared_ptr<BlockTrait> > m_block_traits{};

    };

    template<class T>
    const T* BlockType::query_trait() const
    {
        return const_cast<BlockType*>(this)->query_trait<T>();
    }

    template<class T>
    T* BlockType::query_trait()
    {
        for (const auto& trait : m_block_traits)
        {
            if (T* right_trait = dynamic_cast<T*>(trait.get()))
                return right_trait;
        }

        return nullptr;
    }
}
