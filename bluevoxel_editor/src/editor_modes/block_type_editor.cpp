#include "block_type_editor.h"

#include "block_editor_workspace.h"
#include "core/engine.h"
#include "input/input_component.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_rendering/voxel_rendering_module.h"

#include <fmt/core.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace bluevoxel
{
    BlockTypeEditor::BlockTypeEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_workspace(&workspace)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_ui_module(&g_engine->get_module_checked<h2o::UIModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        // This should be updatable on-demand
        update_preset_names();

        set_tick_phases(h2o::TickPhase::Update);
    }

    void BlockTypeEditor::on_selected_block_changed()
    {

    }

    void BlockTypeEditor::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return;

        u32 selected_block_id = m_workspace->selected_block_id();

        if (ImGui::Begin("Block Type Editor"))
        {
            if (ImGui::Button("Create Block"))
            {
                m_workspace->select_block(voxel_pack->create_block_type("new_block"));

                m_selected_block_name_edit = "new_block";
                ImGui::OpenPopup("Name Selected Block");
            }

            // Block type picker
            if (ImGui::Combo("Block Type", (i32*) (&selected_block_id), m_block_type_names_c_str.data(),
                m_block_type_names_c_str.size()))
                m_workspace->select_block(selected_block_id);

            if (auto edited_block_type = voxel_pack->block_types()[selected_block_id]; edited_block_type->block_id != 0)
            {
                ImGui::Text("ID: %i", selected_block_id);

                if (ImGui::Button(fmt::format("Delete '{}'", edited_block_type->name).c_str()))
                    voxel_pack->delete_block_type(selected_block_id);

                ImGui::SameLine();

                if (ImGui::Button(fmt::format("Rename '{}'", edited_block_type->name).c_str()))
                {
                    m_selected_block_name_edit = edited_block_type->name;
                    ImGui::OpenPopup("Name Selected Block");
                }

                if (ImGui::Combo("Block Preset", (i32*)(&edited_block_type->preset_id),
                    m_block_preset_names_c_str.data(), m_block_preset_names_c_str.size()))
                    voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                if (ImGui::Combo("Block Model", (i32*)(&edited_block_type->model_id), m_block_model_names_c_str.data(),
                    m_block_model_names_c_str.size()))
                    voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (const auto model = voxel_pack->get_uncooked_block_model(edited_block_type->model_id))
                    {
                        for (u32 i = 0; i < model->face_count(); i++)
                        {
                            if (ImGui::Combo(fmt::format("Face {}", i).c_str(), (i32*) (&edited_block_type->texture_ids[i]),
                                m_texture_names_c_str.data(), m_texture_names_c_str.size()))
                                voxel_pack->edit_block_type(selected_block_id, *edited_block_type);
                        }
                    }
                }
            }

            if (ImGui::BeginPopupModal("Name Selected Block"))
            {
                auto edited_block_type = voxel_pack->block_types()[selected_block_id];
                if (!edited_block_type)
                    return;

                ImGui::InputText("New Name", &m_selected_block_name_edit);

                if (ImGui::Button("Confirm"))
                {
                    edited_block_type->name = m_selected_block_name_edit;
                    voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel"))
                    ImGui::CloseCurrentPopup();

                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void BlockTypeEditor::on_voxel_pack_updated()
    {
        update_block_type_names();
        update_model_names();
        update_texture_names();
    }

    void BlockTypeEditor::update_block_type_names()
    {
        m_block_type_names_c_str.clear();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            const auto& block_types = voxel_pack->block_types();
            m_block_type_names_c_str.reserve(block_types.size());

            for (const auto& block_type : block_types)
            {
                if (block_type)
                {
                    m_block_type_names_c_str.push_back(block_type->name.c_str());
                }
                else
                {
                    m_block_type_names_c_str.push_back("none");
                }
            }
        }
    }

    void BlockTypeEditor::update_texture_names()
    {
        m_texture_names_c_str.clear();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            const auto& texture_ids = voxel_pack->texture_ids();

            m_texture_names_c_str.reserve(texture_ids.size());
            for (const auto& texture_path_string : texture_ids)
                m_texture_names_c_str.push_back(texture_path_string.first.c_str());
        }
    }

    void BlockTypeEditor::update_preset_names()
    {
        m_block_preset_names_c_str.clear();

        const auto& block_presets = m_voxel_module->block_presets();

        m_block_preset_names_c_str.reserve(block_presets.size());
        for (const auto& block_preset : block_presets)
            m_block_preset_names_c_str.push_back(block_preset.name.c_str());
    }

    void BlockTypeEditor::update_model_names()
    {
        m_block_model_names_c_str.clear();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            voxel_pack->for_each_uncooked_block_model(
                [&](const h2o::UncookedBlockModel& block_model)
                {
                    m_block_model_names_c_str.push_back(block_model.name.c_str());
                }
            );
        }
    }
}