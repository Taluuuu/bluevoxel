#include "block_editor_workspace.h"

#include "block_model_editor.h"
#include "block_renderer.h"
#include "block_type_editor.h"
#include "core/engine.h"
#include "game_framework/components/third_person_camera_component.h"
#include "input/input_component.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

namespace bluevoxel
{
    BlockEditorWorkspace::BlockEditorWorkspace(h2o::Tickable* owner)
        : Tickable(owner)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_ui_module(&g_engine->get_module_checked<h2o::UIModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        m_scene = std::make_shared<h2o::Scene>("editor_scene", nullptr);
        m_scene->add_system<h2o::RenderingSystem>();

        // Spawn camera
        auto camera = m_scene->spawn_actor();
        camera->add_component<h2o::InputComponent>();
        auto tps_cam = camera->add_component<h2o::ThirdPersonCameraComponent>();
        tps_cam->hold_click_to_rotate = true;
        camera->tag_actor(h2o::ActorTag::LocalPlayer);
        camera->transform.position = { 0.5f, 0.5f, 0.5f };
        camera->transform.rotation = { 0.0f, 0.0f, 90.0f };

        m_block_renderer = std::make_shared<BlockRenderer>();
        m_block_renderer->set_block(h2o::Block{ 0 });

        m_block_type_editor = std::make_shared<BlockTypeEditor>(*this);
        m_block_model_editor = std::make_shared<BlockModelEditor>(*this);

        on_voxel_pack_changed();
        m_voxel_module->on_voxel_pack_changed.add_listener(m_on_voxel_pack_changed_event_handle,
            [this](const h2o::VoxelPackChangedEvent& event)
            {
                on_voxel_pack_changed();
            }
        );

        set_tick_phases(h2o::TickPhase::Update | h2o::TickPhase::Render);
    }

    void BlockEditorWorkspace::select_block(u32 block_id)
    {
        if (m_selected_block_id == block_id)
            return;

        m_selected_block_id = block_id;
        m_block_renderer->set_block(h2o::Block{ static_cast<h2o::BlockID>(block_id) });

        m_block_type_editor->on_selected_block_changed();
    }

    void BlockEditorWorkspace::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();

        m_ui_module->window("Workspace", { { 1175.0f, 25.0f }, { 400.0f, 100.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 2);

                if (!voxel_pack)
                {
                    ui.label("Missing Voxel Pack.");
                    if (ui.button("Refresh"))
                        reload_voxel_pack();
                }
                else
                {
                    if (ui.button("Refresh"))
                        reload_voxel_pack();

                    if (ui.button("Save"))
                        voxel_pack->save();
                }
            }
        );
    }

    void BlockEditorWorkspace::render()
    {
        auto render_system = m_scene->get_system<h2o::RenderingSystem>();
        if (!render_system)
            return;

        const auto& camera = render_system->main_camera();
        if (!camera)
            return;

        auto& renderer = m_rendering_module->renderer();

        // TODO: Avoid calculating proj_view matrix here.
        const m4 proj_view = camera->calc_proj_view();
        renderer.set_proj_view_matrix(proj_view); // TODO: This is a hack to allow drawing debug lines. Rework this ASAP.

        m_block_renderer->render(renderer);

        // Draw gizmo
        renderer.draw_debug_line(v3(-0.5f), { 1.5f,-0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f });
        renderer.draw_debug_line(v3(-0.5f), {-0.5f,-0.5f, 1.5f }, { 0.0f, 0.0f, 1.0f, 1.0f });
        renderer.draw_debug_line(v3(-0.5f), {-0.5f, 1.5f,-0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f });
    }

    void BlockEditorWorkspace::on_voxel_pack_changed()
    {
        on_voxel_pack_updated();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            voxel_pack->on_voxel_pack_updated.add_listener(m_on_voxel_pack_updated_event_handle,
                [this](const h2o::VoxelPackUpdatedEvent& event)
                {
                    on_voxel_pack_updated();
                }
            );
        }
    }

    void BlockEditorWorkspace::on_voxel_pack_updated()
    {
        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            // Make sure the selected block id is valid
            select_block(glm::min(size_t(m_selected_block_id), voxel_pack->block_types().size() - 1));
        }

        m_block_type_editor->on_voxel_pack_updated();
    }

    void BlockEditorWorkspace::reload_voxel_pack()
    {
        if (auto voxel_pack = g_engine->resource_mgr().reload<h2o::VoxelPack>("../Resources/bluevoxel/voxel/"))
            m_voxel_module->set_voxel_pack(voxel_pack);
    }
}