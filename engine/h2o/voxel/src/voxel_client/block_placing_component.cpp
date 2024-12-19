#include "voxel_client/block_placing_component.h"

#include "core/engine.h"
#include "core/utils.h"
#include "input/input_component.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture.h"
#include "scene/scene.h"
#include "scene_rendering/camera_component.h"
#include "voxel/traits/block_trait_rotation.h"
#include "voxel/voxel_ray.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/block_placeable_interface.h"
#include "voxel_client/inventory_manager_voxel.h"

namespace h2o
{
    BlockPlacingComponent::BlockPlacingComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
        , m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<VoxelModule>())
    {
        m_hotbar_inventory = std::make_shared<Inventory<Block>>("PlayerHotbar", 9, 1, std::nullopt);
        m_inventory = std::make_shared<Inventory<Block>>("PlayerInventory", 9, 3, std::nullopt);

        m_voxel_module->inventory_manager()->open(m_hotbar_inventory, InventoryUIAnchor::Bottom);

        // TEMP: Just calissate every block in the inventory
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        for (const auto& block_type : voxel_pack->block_types())
        {
            if (block_type)
                m_inventory->add_item_stack({ .item = Block{block_type->block_id}, .count = 1 });
        }

        m_camera = owner()->get_component<CameraComponent>();
        m_input = owner()->get_component<InputComponent>();

        m_crosshair_texture = g_engine->resource_mgr().fetch<gfx::Texture>("engine/textures/crosshair.png");

        set_tick_phases(TickPhase::PostUpdate);
    }

    void BlockPlacingComponent::post_update(f32 delta_time)
    {
        assert(m_camera);
        assert(m_input);

        if (!block_placeable || !m_hotbar_inventory)
            return;

        // Draw crosshair
        if (m_crosshair_texture)
        {
            const auto draw_list = ImGui::GetBackgroundDrawList();
            const v2 viewport_size = ImGui::GetIO().DisplaySize;

            const v2 crosshair_min = viewport_size / 2.0f - m_crosshair_size / 2.0f;
            const v2 crosshair_max = crosshair_min + m_crosshair_size;
            draw_list->AddImage((void*)(u64)m_crosshair_texture->id(), crosshair_min, crosshair_max);
        }

        // Open/close inventory
        auto& layer_stack = g_engine->layer_stack();
        if (layer_stack.top_layer() <= Layer::Inventory &&
            m_input->key_state(Key::E).pressed_this_frame)
        {
            const auto& inventory_mgr = g_engine->get_module_checked<VoxelModule>().inventory_manager();
            if (layer_stack.has_layer(Layer::Inventory))
            {
                layer_stack.pop_layer(Layer::Inventory);
                inventory_mgr->close(m_inventory);
            }
            else
            {
                layer_stack.push_layer(Layer::Inventory, { false, true });
                inventory_mgr->open(m_inventory, InventoryUIAnchor::Center);
            }
        }

        // Cycle through hotbar slots
        auto& selected_index = m_hotbar_inventory->selected_index;
        selected_index = utils::non_stupid_mod(
            i32(selected_index.value_or(0)) - i32(ImGui::GetIO().MouseWheel),
            i32(m_hotbar_inventory->num_items_per_row()));

        auto& chunk_mgr = block_placeable->chunk_mgr();

        const v3 front = m_camera->camera().front();
        const v3 origin = owner()->transform.position;
        const v3 end = origin + front * 15.0f;

        if (const VoxelRay ray{ origin, end, chunk_mgr })
        {
            const auto& [hit_voxel, before_hit_voxel, normal, distance] = ray.hit();
            auto& renderer = m_rendering_module->renderer();
            constexpr v4 line_color{ 0.0f, 0.0f, 0.0f, 1.0f };
            const v3i pos = hit_voxel.pos;

            // Draw selection highlight
            renderer.draw_cube(v3{pos} - v3{0.005f}, v3{1.01f}, line_color);

            if (true)//layer_stack.top_layer() == Layer::Game)
            {
                if (m_input->mouse_button_state(MouseButton::Left).pressed_this_frame)
                    block_placeable->set_block_at(hit_voxel.pos, Block::Air);

                if (m_input->mouse_button_state(MouseButton::Right).pressed_this_frame)
                {
                    if (const auto selected_item = m_hotbar_inventory->selected_item())
                    {
                        Block block = selected_item->item;

                        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
                        {
                            if (const BlockType* block_type = voxel_pack->get_block_type(selected_item->item.id))
                            {
                                if (const auto rotation = block_type->query_trait<BlockTrait_Rotation>())
                                    block = rotation->rotate_to_normal(block, normal);

                                block_placeable->set_block_at(before_hit_voxel.pos, block);
                            }
                        }
                    }
                }
            }
        }
    }
}
