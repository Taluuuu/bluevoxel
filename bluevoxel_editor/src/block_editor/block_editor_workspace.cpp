#include "block_editor_workspace.h"

#include "block_editor/block_model_editor/block_model_editor.h"
#include "block_renderer.h"
#include "block_editor/block_type_editor/block_type_editor.h"
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

#include <imgui.h>

namespace bluevoxel
{
    BlockEditorWorkspace::BlockEditorWorkspace(h2o::Tickable* owner)
        : Tickable(owner)
        , m_selection_manager(this)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        m_scene = std::make_shared<h2o::Scene>("editor_scene", nullptr);
        m_scene->add_system<h2o::RenderingSystem>();

        // Spawn camera
        auto camera = m_scene->spawn_actor();
        camera->add_component<h2o::InputComponent>();
        auto tps_cam = camera->add_component<h2o::ThirdPersonCameraComponent>();
        tps_cam->rotate_mouse_button = h2o::MouseButton::Right;
        camera->tag_actor(h2o::ActorTag::LocalPlayer);
        camera->transform.position = { 0.5f, 0.5f, 0.5f };
        camera->transform.rotation = { 0.0f, 0.0f, 0.0f };

        m_block_renderer = std::make_shared<BlockRenderer>(*this);
        m_block_renderer->set_block(h2o::Block{ 0 });

        m_block_type_editor = std::make_shared<BlockTypeEditor>(*this);
        m_block_model_editor = std::make_shared<BlockModelEditor>(*this);
        m_block_model_editor->set_enabled(false);

        on_voxel_pack_changed();
        m_voxel_module->on_voxel_pack_changed.add_listener(m_on_voxel_pack_changed_event_handle,
            [this](const h2o::VoxelPackChangedEvent& event)
            {
                on_voxel_pack_changed();
            }
        );

        set_tick_phases(h2o::TickPhase::Update | h2o::TickPhase::Render);
    }

    void BlockEditorWorkspace::select_block(const u32 block_id)
    {
        m_selected_block_id = block_id;
        m_block_renderer->set_block(h2o::Block{ static_cast<h2o::BlockID>(block_id) });

        m_block_type_editor->on_selected_block_changed();
    }

    void BlockEditorWorkspace::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::BeginDisabled(voxel_pack == nullptr);
                if (ImGui::MenuItem("Save"))
                    voxel_pack->save();
                ImGui::EndDisabled();

                if (ImGui::MenuItem("Reload Voxel Pack"))
                    reload_voxel_pack();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Show Block Model Editor", nullptr, m_block_model_editor->is_enabled()))
                    m_block_model_editor->set_enabled(!m_block_model_editor->is_enabled());

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
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

        // Draw gizmo
        renderer.draw_line(v3(-0.5f), {1.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f});
        renderer.draw_line(v3(-0.5f), {-0.5f, -0.5f, 1.5f}, {0.0f, 0.0f, 1.0f, 1.0f});
        renderer.draw_line(v3(-0.5f), {-0.5f, 1.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f});
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
        if (auto voxel_pack = g_engine->resource_mgr().reload<h2o::VoxelPack>("bluevoxel/voxel/"))
            m_voxel_module->set_voxel_pack(voxel_pack);
    }
}