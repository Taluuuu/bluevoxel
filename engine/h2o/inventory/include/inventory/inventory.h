#pragma once

#include "item_stack.h"

#include <optional>
#include <vector>

namespace h2o
{
    // TODO: Create class hotbar deriving from Inventory

    template<class ItemType>
    class Inventory
    {
    public:

        // TODO: Add num items per stack limit
        Inventory(
            std::optional<u32> num_item_slots,
            std::optional<u32> max_item_count);

        void set_num_item_slots(std::optional<u32> num_item_slots);
        void set_max_item_count(std::optional<u32> max_item_count);

        // Add the item stack to the inventory; returns the number of items that could not be stored.
        u32 add_item_stack(const ItemStack<ItemType>& item_stack);
        bool set_item_stack(u32 stack_index, const ItemStack<ItemType>& item_stack);
        std::optional< ItemStack<ItemType> > remove_stack(u32 stack_index);

        [[nodiscard]] u32 compute_num_items() const;
        [[nodiscard]] const std::vector< std::optional< ItemStack<ItemType> > >& item_stacks() const { return m_item_stacks; }

    private:

        // The max number of stored item stacks
        std::optional<u32> m_num_item_slots = std::nullopt;

        // The max number of stored individual items
        std::optional<u32> m_max_item_count = std::nullopt;

        std::vector< std::optional< ItemStack<ItemType> > > m_item_stacks{};

    };

    template<class ItemType>
    Inventory<ItemType>::Inventory(
        std::optional<u32> num_item_slots,
        std::optional<u32> max_item_count)
    {
        set_num_item_slots(num_item_slots);
        set_max_item_count(max_item_count);
    }

    template<class ItemType>
    void Inventory<ItemType>::set_num_item_slots(std::optional<u32> num_item_slots)
    {
        m_num_item_slots = num_item_slots;

        if (num_item_slots)
            m_item_stacks.resize(*num_item_slots);
    }

    template<class ItemType>
    void Inventory<ItemType>::set_max_item_count(std::optional<u32> max_item_count)
    {
        m_max_item_count = max_item_count;
    }

    template<class ItemType>
    u32 Inventory<ItemType>::add_item_stack(const ItemStack<ItemType>& item_stack)
    {
        // TODO: Do item count validations
        for (auto& stack : m_item_stacks)
        {
            if (!stack)
            {
                stack = item_stack;
                return 0;
            }

            if (stack->item == item_stack.item)
            {
                stack->count += item_stack.count;
                return 0;
            }
        }

        if (!m_num_item_slots)
        {
            m_item_stacks.push_back(item_stack);
            return 0;
        }

        assert(*m_num_item_slots == m_item_stacks.size());
        return item_stack.count;
    }

    template<class ItemType>
    bool Inventory<ItemType>::set_item_stack(
        u32 stack_index,
        const ItemStack<ItemType>& item_stack)
    {
        if (stack_index >= m_item_stacks.size())
            return false;

        if (m_item_stacks[stack_index])
            return false; // There is already an item stack there

        m_item_stacks[stack_index] = item_stack;
        return true;
    }

    template<class ItemType>
    std::optional< ItemStack<ItemType> > Inventory<ItemType>::remove_stack(u32 stack_index)
    {
        if (stack_index >= m_item_stacks.size())
            return std::nullopt;

        auto item_stack = m_item_stacks[stack_index];
        m_item_stacks[stack_index] = std::nullopt;

        return item_stack;
    }

    template<class ItemType>
    u32 Inventory<ItemType>::compute_num_items() const
    {
        u32 num_items = 0;
        for (const auto& stack : m_item_stacks)
            num_items += stack ? stack->count : 0;

        return num_items;
    }
}