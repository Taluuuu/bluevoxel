#pragma once

#include "core/tickable.h"
#include "inventory/inventory.h"
#include "inventory/inventory_draw_data.h"
#include "ui/imgui.h"

#include <fmt/core.h>
#include <memory>

namespace h2o
{
    template<class ItemType>
    class InventoryUI : public Tickable
    {
    public:

        using InvDrawDataRef = std::shared_ptr<InventoryDrawData<ItemType>>;

        explicit InventoryUI(Tickable* owner, const InvDrawDataRef& draw_Data);
        ~InventoryUI() override = default;

    public:

        std::weak_ptr< Inventory<ItemType> > weak_inventory{};

    protected:

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        std::optional< ItemStack<ItemType> > m_selected_stack = std::nullopt;

        InvDrawDataRef m_draw_data = nullptr;

    };

    template<class ItemType>
    InventoryUI<ItemType>::InventoryUI(Tickable* owner, const InvDrawDataRef& draw_Data)
        : Tickable(owner)
        , m_draw_data(draw_Data)
    {
        set_tick_phases(TickPhase::Update);
    }

    template<class ItemType>
    void InventoryUI<ItemType>::update(f32 delta_time)
    {
        auto inventory = weak_inventory.lock();
        if (!inventory || !m_draw_data)
            return;

        const v2 mouse_pos = ImGui::GetMousePos();
        auto foreground_draw_list = ImGui::GetForegroundDrawList();
        if (m_selected_stack)
        {
            if (const auto tex_id = m_draw_data->fetch_item_texture_id(m_selected_stack->item))
            {
                foreground_draw_list->AddImage(
                    (void*)(u64)(*tex_id),
                    mouse_pos - v2{ m_draw_data->item_texture_size } / 2.0f,
                    mouse_pos + v2{ m_draw_data->item_texture_size } / 2.0f);
            }
        }

        if (ImGui::Begin("Inventory"))
        {
            const v2 window_size = ImGui::GetWindowSize();

            auto& item_stacks = inventory->item_stacks();

            const i32 num_columns = window_size.x /
                (m_draw_data->item_texture_size + m_draw_data->item_texture_spacing);

            if (num_columns > 0)
            {
                const i32 num_rows = glm::ceil(item_stacks.size() / f32(num_columns));

                const v2 grid_pos = ImGui::GetCursorScreenPos();
                auto draw_list = ImGui::GetWindowDrawList();
                for (u32 i = 0; i < num_columns; i++)
                for (u32 j = 0; j < num_rows; j++)
                {
                    u32 stack_index = i + num_columns * j;
                    if (stack_index >= item_stacks.size())
                        break;

                    const v2 tex_offset{
                        i * (m_draw_data->item_texture_size + m_draw_data->item_texture_spacing),
                        j * (m_draw_data->item_texture_size + m_draw_data->item_texture_spacing) };

                    const v2 item_pos = grid_pos + tex_offset;

                    ImGui::SetCursorScreenPos(item_pos);

                    if (ImGui::InvisibleButton(
                        fmt::format("Item({};{})", i, j).c_str(),
                        v2{ m_draw_data->item_texture_size, m_draw_data->item_texture_size },
                        ImGuiButtonFlags_MouseButtonLeft))
                    {
                        // Item slot is clicked
                        const auto item_stack = m_selected_stack;
                        m_selected_stack = inventory->remove_stack(stack_index);

                        if (item_stack)
                            inventory->set_item_stack(stack_index, *item_stack);
                    }

                    if (const auto& item_stack = item_stacks[stack_index])
                    {
                        if (const auto tex_id = m_draw_data->fetch_item_texture_id(item_stack->item))
                        {
                            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                                ImGui::SetTooltip("%s", m_draw_data->fetch_item_name(item_stack->item).c_str());

                            draw_list->AddImage(
                                (void*)(u64)(*tex_id),
                                item_pos, item_pos + v2{ m_draw_data->item_texture_size });

                            draw_list->AddText(
                                item_pos,
                                ImColor(1.0f, 1.0f, 1.0f, 1.0f),
                                fmt::format("x{}", item_stack->count).c_str());
                        }
                    }

                    // Blank space...
                    draw_list->AddRect(
                        grid_pos + tex_offset,
                        grid_pos + tex_offset + v2{ m_draw_data->item_texture_size },
                        ImColor(0.5f, 0.7f, 1.0f, 1.0f), 0.0f, 0, 2.0f);
                }
            }
        }

        ImGui::End();
    }
}
