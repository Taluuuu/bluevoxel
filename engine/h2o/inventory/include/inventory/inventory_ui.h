#pragma once

#include "core/tickable.h"
#include "inventory/inventory.h"
#include "ui/imgui.h"

#include <fmt/core.h>

namespace h2o
{
    template<class ItemType>
    class InventoryUI : public Tickable
    {
    public:

        explicit InventoryUI(Tickable* owner);
        ~InventoryUI() override = default;

        // TODO: Use a shared_ptr to make sure the inventory does not get deallocated
        void open(const Inventory<ItemType>& inventory);
        void close();

    public:

        f32 item_texture_size = 64.0f;
        f32 item_texture_spacing = 8.0f;

    protected:

        void update(f32 delta_time) override;

        [[nodiscard]] virtual std::optional<u32> fetch_item_texture_id(const ItemType& item) const = 0;
        [[nodiscard]] virtual std::string fetch_item_name(const ItemType& item) const = 0;

    private:

        const Inventory<ItemType>* m_inventory = nullptr;

    };

    template<class ItemType>
    void InventoryUI<ItemType>::open(const Inventory<ItemType>& inventory)
    {
        m_inventory = &inventory;
    }

    template<class ItemType>
    void InventoryUI<ItemType>::close()
    {
        m_inventory = nullptr;
    }

    template<class ItemType>
    InventoryUI<ItemType>::InventoryUI(Tickable* owner)
        : Tickable(owner)
    {
        set_tick_phases(TickPhase::Update);
    }

    template<class ItemType>
    void InventoryUI<ItemType>::update(f32 delta_time)
    {
        if (!m_inventory)
            return;

        if (ImGui::Begin("Inventory"))
        {
            const v2 window_size = ImGui::GetWindowSize();

            const auto& item_stacks = m_inventory->item_stacks();
            const i32 num_columns = window_size.x / (item_texture_size + item_texture_spacing);
            if (num_columns > 0)
            {
                const i32 num_rows = glm::ceil(item_stacks.size() / f32(num_columns));

                const v2 grid_pos = ImGui::GetCursorScreenPos();
                auto draw_list = ImGui::GetWindowDrawList();
                for (u32 i = 0; i < num_columns; i++)
                {
                    for (u32 j = 0; j < num_rows; j++)
                    {
                        u32 stack_index = i + num_columns * j;
                        if (stack_index >= item_stacks.size())
                            break;

                        if (const auto& item_stack = item_stacks[stack_index])
                        {
                            if (const auto tex_id = fetch_item_texture_id(item_stack->item))
                            {
                                // Draw texture
                                const v2 tex_offset{
                                    i * (item_texture_size + item_texture_spacing),
                                    j * (item_texture_size + item_texture_spacing) };

                                const v2 item_pos = grid_pos + tex_offset;

                                ImGui::SetCursorScreenPos(item_pos);
                                ImGui::InvisibleButton(
                                    fmt::format("Item({};{})", i, j).c_str(),
                                    v2{ item_texture_size, item_texture_size },
                                    ImGuiButtonFlags_MouseButtonLeft);

                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                                    ImGui::SetTooltip("%s", fetch_item_name(item_stack->item).c_str());

                                draw_list->AddImage(
                                    (void*)(u64)(*tex_id),
                                    item_pos, item_pos + v2{ item_texture_size });

                                draw_list->AddText(item_pos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), fmt::format("x{}", item_stack->count).c_str());

                                continue;
                            }
                        }

                        // Blank space...
                    }
                }
            }
        }

        ImGui::End();
    }
}