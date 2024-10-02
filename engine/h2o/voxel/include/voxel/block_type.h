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

        // NOTE: Should traits be managed in a separate class ?
        // Returns true on success
        template<class T, class... Args>
        bool add_trait(Args... args);

        template<class T> [[nodiscard]] const T* query_trait() const;
        template<class T> [[nodiscard]] T* query_trait();

        void for_each_trait(const std::function<void(const BlockTrait&)>& function) const;

    public:

        std::string name{};
        BlockID block_id = 0;

        std::vector<u32> texture_ids{};
        u32 model_id = 0;

        bool is_transparent = false;

    private:

        // Calculate the number of bits in the data field currently in use
        [[nodiscard]] u32 calc_trait_bit_count() const;

    private:

        std::vector< std::shared_ptr<BlockTrait> > m_block_traits{};

    };

    template<class T, class... Args>
    bool BlockType::add_trait(Args... args)
    {
        static_assert(std::is_base_of_v<BlockTrait, T>, "T must derive from h2o::BlockTrait");

        const u32 current_bit_count = calc_trait_bit_count();
        auto new_trait = std::make_shared<T>(*this, current_bit_count, args...);

        if (current_bit_count + new_trait->num_bits() > max_data_bits)
        {
            log::warn("Trying to add too many data bits to block type: {}", name);
            return false;
        }

        m_block_traits.emplace_back(std::move(new_trait));
        return true;
    }

    template<class T>
    const T* BlockType::query_trait() const
    {
        return const_cast<T*>(this)->template query_trait<T>();
    }

    template<class T>
    T* BlockType::query_trait()
    {
        for (const auto& trait : m_block_traits)
        {
            if (T* right_trait = dynamic_cast<T>(trait.get()))
                return right_trait;
        }

        return nullptr;
    }
}
