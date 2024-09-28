#pragma once

#include "core/engine.h"
#include "core/tickable.h"
#include "core/types.h"
#include "inventory.h"
#include "item_stack.h"
#include "ui/imgui.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

namespace h2o
{
    enum class InventoryUIAnchor
    {
        Center,
        Bottom,
        Free,
    };

    template<class ItemType>
    class InventoryManager_Base : public Tickable
    {
    public:

        explicit InventoryManager_Base(Tickable* owner);
        ~InventoryManager_Base() override = default;

        [[nodiscard]] virtual std::optional<u32> fetch_item_texture_id(const ItemType& item) const = 0;
        [[nodiscard]] virtual std::string fetch_item_name(const ItemType& item) const = 0;

        using InvRef = std::shared_ptr<Inventory<ItemType>>;
        void open(const InvRef& inventory, InventoryUIAnchor anchor);
        void close(const InvRef& inventory);

    public:

        f32 item_texture_size = 64.0f;
        f32 item_texture_spacing = 8.0f;

        // The item currently attached to the mouse cursor
        std::optional< ItemStack<ItemType> > held_item_stack = std::nullopt;

    protected:

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        void draw_held_item_stack();
        void draw_inventory(Inventory<ItemType>& inventory, InventoryUIAnchor anchor);

    private:

        struct OpenInventoryData
        {
            InvRef inventory = nullptr;
            InventoryUIAnchor anchor = InventoryUIAnchor::Center;
        };

        std::vector<OpenInventoryData> m_opened_inventories{};

    };

    template<class ItemType>
    InventoryManager_Base<ItemType>::InventoryManager_Base(Tickable* owner)
        : Tickable(owner)
    {
        set_tick_phases(TickPhase::Update);
    }

    template<class ItemType>
    void InventoryManager_Base<ItemType>::open(const InvRef& inventory, const InventoryUIAnchor anchor)
    {
        const auto it = std::find_if(m_opened_inventories.begin(), m_opened_inventories.end(),
            [&](const OpenInventoryData& inv_data)
            {
                return inv_data.inventory == inventory;
            }
        );

        if (it == m_opened_inventories.end())
            m_opened_inventories.emplace_back(inventory, anchor);
    }

    template<class ItemType>
    void InventoryManager_Base<ItemType>::close(const InvRef& inventory)
    {
        erase_if(m_opened_inventories,
            [&](const OpenInventoryData& inv_data)
            {
                return inv_data.inventory == inventory;
            }
        );
    }

    template<class ItemType>
    void InventoryManager_Base<ItemType>::update(f32 delta_time)
    {
        if (g_engine->layer_stack().top_layer() == Layer::Inventory)
            draw_held_item_stack();

        for (auto& [inventory, anchor] : m_opened_inventories)
            draw_inventory(*inventory, anchor);
    }

    template<class ItemType>
    void InventoryManager_Base<ItemType>::draw_held_item_stack()
    {
        if (!held_item_stack)
            return;

        const auto tex_id = fetch_item_texture_id(held_item_stack->item);
        if (!tex_id)
            return;

        const v2 mouse_pos = ImGui::GetMousePos();
        const auto foreground_draw_list = ImGui::GetForegroundDrawList();

        foreground_draw_list->AddImage(
            (void*)(u64)(*tex_id),
            mouse_pos - v2{ item_texture_size } / 2.0f,
            mouse_pos + v2{ item_texture_size } / 2.0f);
    }

    template<class ItemType>
    void InventoryManager_Base<ItemType>::draw_inventory(
        Inventory<ItemType>& inventory,
        const InventoryUIAnchor anchor)
    {
        const v2u inventory_size {
            inventory.num_items_per_row(),
            inventory.current_row_count()
        };

        const v2 window_size {
            f32(inventory_size.x) * item_texture_size + f32(inventory_size.x + 1) * item_texture_spacing,
            f32(inventory_size.y) * item_texture_size + f32(inventory_size.y + 1) * item_texture_spacing
        };

        ImGuiWindowFlags window_flags =
            // ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar;

        // Setup non-free moving inventory window
        if (anchor != InventoryUIAnchor::Free)
        {
            window_flags = window_flags |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoTitleBar;

            const v2 viewport_size = ImGui::GetIO().DisplaySize;

            v2 window_corner{};
            switch (anchor)
            {
            case InventoryUIAnchor::Bottom:
                window_corner = v2{
                    viewport_size.x / 2.0f - window_size.x / 2.0f,
                    viewport_size.y - window_size.y - item_texture_spacing
                };
                break;

            case InventoryUIAnchor::Center:
                window_corner = v2{
                    viewport_size.x / 2.0f - window_size.x / 2.0f,
                    viewport_size.y / 2.0f - window_size.y / 2.0f
                };
                break;

            default:
                break;
            }

            ImGui::SetNextWindowPos(window_corner);
        }

        ImGui::SetNextWindowSize(window_size);

        if (ImGui::Begin(inventory.name().c_str(), nullptr, window_flags))
        {
            ImDrawList* const draw_list = ImGui::GetWindowDrawList();

            const u32 row_count = inventory.current_row_count();
            const u32 col_count = inventory.num_items_per_row();

            const v2 window_origin = ImGui::GetCursorScreenPos();

            for (u32 i = 0; i < col_count; i++)
            for (u32 j = 0; j < row_count; j++)
            {
                const v2i slot{ i, j };
                const u32 slot_index = inventory.make_stack_index(slot);
                const v2 slot_offset = v2(slot) * (item_texture_size + item_texture_spacing);

                ImGui::SetCursorScreenPos(window_origin + slot_offset);

                if (ImGui::InvisibleButton(
                    fmt::format("Item({};{})", i, j).c_str(),
                    v2{ item_texture_size },
                    ImGuiButtonFlags_MouseButtonLeft))
                {
                    // Item slot is clicked
                    const auto item_stack = held_item_stack;
                    held_item_stack = inventory.remove_stack(slot_index);

                    if (item_stack)
                        inventory.set_item_stack(slot_index, *item_stack);
                }

                const v2 slot_min = window_origin + slot_offset;
                const v2 slot_max = slot_min + v2{ item_texture_size };

                if (const auto& item_stack = inventory.get_item_stack(slot_index))
                {
                    if (const auto tex_id = fetch_item_texture_id(item_stack->item))
                    {
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                            ImGui::SetTooltip("%s", fetch_item_name(item_stack->item).c_str());

                        draw_list->AddImage((void*)(u64)(*tex_id), slot_min, slot_max);
                        draw_list->AddText(
                            slot_min,
                            ImColor(1.0f, 1.0f, 1.0f, 1.0f),
                            fmt::format("x{}", item_stack->count).c_str());
                    }
                }

                // Draw item frame
                ImColor frame_color = ImColor(0.5f, 0.7f, 1.0f, 1.0f);
                f32 frame_thickness = 2.0f;
                if (slot_index == inventory.selected_index)
                {
                    frame_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
                    frame_thickness = 6.0f;
                }

                draw_list->AddRect(slot_min, slot_max, frame_color, 0.0f, 0, frame_thickness);
            }
        }

        ImGui::End();
    }
}
