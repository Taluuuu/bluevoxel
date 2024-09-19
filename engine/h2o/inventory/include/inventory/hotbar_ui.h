#pragma once

#include "core/tickable.h"
#include "core/utils.h"
#include "inventory.h"
#include "inventory_draw_data.h"
#include "ui/imgui.h"

#include <memory>

namespace h2o
{
    template<class ItemType>
    class HotbarUI : public Tickable
    {
    public:

        using InvDrawDataRef = std::shared_ptr<InventoryDrawData<ItemType>>;

        HotbarUI(Tickable* owner, const InvDrawDataRef& draw_data);
        ~HotbarUI() override = default;

    public:

        std::weak_ptr< Inventory<ItemType> > weak_inventory{};

    protected:

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        InvDrawDataRef m_draw_data = nullptr;

        i32 m_selection_index = 0;

    };

    template<class ItemType>
    HotbarUI<ItemType>::HotbarUI(Tickable* owner, const InvDrawDataRef& draw_data)
        : Tickable(owner)
        , m_draw_data(draw_data)
    {
        set_tick_phases(TickPhase::Update);
    }

    template<class ItemType>
    void HotbarUI<ItemType>::update(f32 delta_time)
    {
        auto inventory = weak_inventory.lock();
        if (!inventory)
            return;

        const auto& item_stacks = inventory->item_stacks();
        if (item_stacks.empty())
            return;

        const i32 num_item_stacks = item_stacks.size();
        m_selection_index = utils::non_stupid_mod(m_selection_index - i32(ImGui::GetIO().MouseWheel), num_item_stacks);

        const v2 hotbar_size {
            num_item_stacks * m_draw_data->item_texture_size +
                (num_item_stacks - 1) * m_draw_data->item_texture_spacing,
            m_draw_data->item_texture_size
        };

        const v2 window_size = ImGui::GetIO().DisplaySize;

        const v2 hotbar_corner  = {
            window_size.x / 2.0f - hotbar_size.x / 2.0f,
            window_size.y - hotbar_size.y - m_draw_data->item_texture_spacing
        };

        ImDrawList* const draw_list = ImGui::GetForegroundDrawList();

        const auto bg_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        draw_list->AddRectFilled(
            hotbar_corner - v2{m_draw_data->item_texture_spacing},
            hotbar_corner + hotbar_size + v2{m_draw_data->item_texture_spacing},
            ImColor(bg_color), 0.0f, 0
        );

        for (i32 i = 0; i < num_item_stacks; i++)
        {
            const v2 item_min = hotbar_corner + v2{ i, 0.0f } * (m_draw_data->item_texture_size + m_draw_data->item_texture_spacing);
            const v2 item_max = item_min + v2{ m_draw_data->item_texture_size };

            if (const auto& item = item_stacks[i])
            {
                if (const auto tex_id = m_draw_data->fetch_item_texture_id(item->item))
                    draw_list->AddImage((void*)(u64)(*tex_id), item_min, item_max);
            }

            ImColor frame_color = ImColor(0.5f, 0.7f, 1.0f, 1.0f);
            f32 frame_thickness = 2.0f;
            if (i == m_selection_index)
            {
                frame_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
                frame_thickness = 6.0f;
            }

            draw_list->AddRect(item_min, item_max, frame_color, 0.0f, 0, frame_thickness);
        }
    }
}
