#include "voxel_client/block_placing_component.h"

#include "core/engine.h"
#include "input/input_component.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/camera_component.h"
#include "voxel/voxel_ray.h"
#include "voxel/voxel_module.h"
#include "voxel_client/block_placeable_interface.h"
#include "voxel_client/voxel_inventory_draw_data.h"

namespace h2o
{
    BlockPlacingComponent::BlockPlacingComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
        , m_hotbar_ui(this, g_engine->get_module_checked<VoxelModule>().inventory_draw_data())
        , m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
    {
        m_hotbar_inventory = std::make_shared< Inventory<Block> >(9, std::nullopt);
        m_hotbar_ui.weak_inventory = m_hotbar_inventory;

        m_hotbar_inventory->add_item_stack({ .item = Block{1}, .count = 1 });
        m_hotbar_inventory->add_item_stack({ .item = Block{2}, .count = 1 });
        m_hotbar_inventory->add_item_stack({ .item = Block{3}, .count = 1 });
        m_hotbar_inventory->add_item_stack({ .item = Block{4}, .count = 1 });
        m_hotbar_inventory->add_item_stack({ .item = Block{5}, .count = 1 });
        m_hotbar_inventory->add_item_stack({ .item = Block{6}, .count = 1 });
        m_hotbar_inventory->add_item_stack({ .item = Block{7}, .count = 1 });

        m_camera = owner()->get_component<CameraComponent>();
        m_input = owner()->get_component<InputComponent>();

        set_tick_phases(TickPhase::Update);
    }

    void BlockPlacingComponent::update(f32 delta_time)
    {
        assert(m_camera);
        assert(m_input);

        if (!block_placeable)
            return;

        const auto& chunk_mgr = block_placeable->chunk_mgr();

        const v3 front = m_camera->camera().front();
        const v3 origin = owner()->transform.position;
        const v3 end = origin + front * 15.0f;

        if (const VoxelRay ray{ origin, end, chunk_mgr })
        {
            const auto& [hit_voxel, before_hit_voxel] = ray.hit();
            auto& renderer = m_rendering_module->renderer();
            const v4 line_color{ 0.0f, 0.0f, 0.0f, 1.0f };
            const v3i pos = hit_voxel.pos;

            // Draw selection highlight
            renderer.draw_cube(v3{pos} - v3{0.005f}, v3{1.01f}, line_color);

            if (g_engine->layer_stack().top_layer() == Layer::Game)
            {
                if (m_input->mouse_button_state(MouseButton::Left).pressed_this_frame)
                    block_placeable->set_block_at(hit_voxel.pos, Block::Air);

                if (const auto selected_item = m_hotbar_ui.selected_item())
                {
                    if (m_input->mouse_button_state(MouseButton::Right).pressed_this_frame)
                        block_placeable->set_block_at(before_hit_voxel.pos, selected_item->item);
                }
            }
        }
    }
}