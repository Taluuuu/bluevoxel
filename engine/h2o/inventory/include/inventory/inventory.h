#pragma once

#include "item_stack.h"

#include <optional>
#include <string>
#include <vector>

namespace h2o
{
    template<class ItemType>
    class Inventory
    {
    public:

        // TODO: Add num items per stack limit
        Inventory(
            const std::string& name,
            u32 num_items_per_row,
            std::optional<u32> num_rows,
            std::optional<u32> max_item_count);

        // Add the item stack to the inventory; returns the number of items that could not be stored.
        u32 add_item_stack(const ItemStack<ItemType>& item_stack);
        bool set_item_stack(u32 stack_index, const ItemStack<ItemType>& item_stack);
        std::optional< ItemStack<ItemType> > remove_stack(u32 stack_index);
        [[nodiscard]] std::optional< ItemStack<ItemType> > get_item_stack(u32 stack_index) const;

        [[nodiscard]] bool is_valid_index(u32 slot_index) const;
        [[nodiscard]] u32 make_stack_index(v2u stack_pos) const;
        [[nodiscard]] u32 compute_num_items() const;

        [[nodiscard]] u32 current_row_count() const;
        [[nodiscard]] u32 num_items_per_row() const { return m_num_items_per_row; }

        [[nodiscard]] const std::string& name() const { return m_name; }

        [[nodiscard]] std::optional< ItemStack<ItemType> > selected_item() const;

    public:

        std::optional<u32> selected_index = std::nullopt;

    private:

        // Returns true on success
        bool resize_to_fit(u32 slot_index);

    private:

        u32 m_num_items_per_row = 0;                  // X
        std::optional<u32> m_num_rows = std::nullopt; // Y - Unlimited if null

        // The max number of stored individual items
        std::optional<u32> m_max_item_count = std::nullopt;

        std::vector< std::optional<ItemStack<ItemType>> > m_item_stacks{};

        std::string m_name = "";

    };

    template<class ItemType>
    Inventory<ItemType>::Inventory(
        const std::string& name,
        const u32 num_items_per_row,
        const std::optional<u32> num_rows,
        const std::optional<u32> max_item_count)
        : m_num_items_per_row(num_items_per_row)
        , m_num_rows(num_rows)
        , m_max_item_count(max_item_count)
        , m_name(name)
    {}

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

        const u32 new_stack_index = m_item_stacks.size();
        if (resize_to_fit(new_stack_index))
        {
            m_item_stacks[new_stack_index] = item_stack;
            return 0;
        }

        return item_stack.count;
    }

    template<class ItemType>
    bool Inventory<ItemType>::set_item_stack(
        u32 stack_index,
        const ItemStack<ItemType>& item_stack)
    {
        if (!resize_to_fit(stack_index))
            return false;

        if (m_item_stacks[stack_index])
            return false; // There is already an item stack there

        m_item_stacks[stack_index] = item_stack;
        return true;
    }

    template<class ItemType>
    std::optional< ItemStack<ItemType> > Inventory<ItemType>::remove_stack(u32 stack_index)
    {
        if (!is_valid_index(stack_index))
            return std::nullopt;

        if (stack_index >= m_item_stacks.size())
            return std::nullopt;

        const auto item_stack = m_item_stacks[stack_index];
        m_item_stacks[stack_index] = std::nullopt;

        return item_stack;
    }

    template<class ItemType>
    std::optional< ItemStack<ItemType> > Inventory<ItemType>::get_item_stack(u32 stack_index) const
    {
        if (stack_index < m_item_stacks.size())
            return m_item_stacks[stack_index];

        return std::nullopt;
    }

    template<class ItemType>
    bool Inventory<ItemType>::is_valid_index(u32 slot_index) const
    {
        if (!m_num_rows.has_value())
            return true;

        return slot_index < *m_num_rows * m_num_items_per_row;
    }

    template<class ItemType>
    u32 Inventory<ItemType>::make_stack_index(v2u stack_pos) const
    {
        return stack_pos.y * m_num_items_per_row + stack_pos.x;
    }

    template<class ItemType>
    u32 Inventory<ItemType>::compute_num_items() const
    {
        u32 num_items = 0;
        for (const auto& stack : m_item_stacks)
            num_items += stack ? stack->count : 0;

        return num_items;
    }

    template<class ItemType>
    u32 Inventory<ItemType>::current_row_count() const
    {
        if (m_num_rows)
            return *m_num_rows;

        return m_item_stacks.size() / m_num_items_per_row;
    }

    template<class ItemType>
    std::optional<ItemStack<ItemType>> Inventory<ItemType>::selected_item() const
    {
        if (selected_index)
            return get_item_stack(*selected_index);

        return std::nullopt;
    }

    template<class ItemType>
    bool Inventory<ItemType>::resize_to_fit(u32 slot_index)
    {
        if (slot_index < m_item_stacks.size())
            return true;

        if (is_valid_index(slot_index))
        {
            m_item_stacks.resize(slot_index + 1);
            return true;
        }

        return false;
    }
}
