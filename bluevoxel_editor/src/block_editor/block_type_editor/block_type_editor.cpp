#include "block_type_editor.h"

#include "block_editor/block_editor_workspace.h"
#include "core/engine.h"
#include "input/input_component.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "ui/imgui.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

#include "fmt/core.h"

namespace bluevoxel
{
    BlockTypeEditor::BlockTypeEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_workspace(&workspace)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
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

            if (auto edited_block_type = voxel_pack->block_types()[selected_block_id];
                edited_block_type && edited_block_type->block_id != 0)
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

                ImGui::Columns(2, nullptr, false);

                ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.8f);

                if (ImGui::Combo("Block Model", (i32*)(&edited_block_type->model_id), m_block_model_names_c_str.data(),
                    m_block_model_names_c_str.size()))
                    voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                ImGui::NextColumn();

                if (ImGui::Button("+"))
                {
                    m_new_block_model_name_edit = "new_model";
                    ImGui::OpenPopup("Name New Model");
                }

                ImGui::Columns();

                if (ImGui::Checkbox("Is Transparent", &edited_block_type->is_transparent))
                    voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (const auto model = voxel_pack->get_uncooked_block_model(edited_block_type->model_id))
                    {
                        for (u32 i = 0; i < model->face_count(); i++)
                        {
                            if (ImGui::Combo(fmt::format("Face {}", i).c_str(), (i32*)(&edited_block_type->texture_ids[i]),
                                m_texture_names_c_str.data(), m_texture_names_c_str.size()))
                                voxel_pack->edit_block_type(selected_block_id, *edited_block_type);
                        }
                    }
                }

                ImGui::BeginGroup();
                ImGui::Text("Block Traits");
                auto& trait_mgr = m_voxel_module->block_trait_manager();
                for (const auto& trait_name : trait_mgr.trait_names())
                {
                    bool has_trait = edited_block_type->has_trait(trait_name);
                    if (ImGui::Checkbox(trait_name.c_str(), &has_trait))
                    {
                        if (has_trait)
                        {
                            trait_mgr.add_trait_to_block_type(trait_name, *edited_block_type);
                        }
                        else
                        {
                            edited_block_type->remove_trait(trait_name);
                        }

                        // TODO: This sucks
                        voxel_pack->edit_block_type(selected_block_id, *edited_block_type);
                    }
                }
                ImGui::EndGroup();
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), v2{ ImGui::GetWindowWidth(), ImGui::GetItemRectMax().y },
                    ImColor(ImGui::GetStyle().Colors[ImGuiCol_Border]));
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

            if (ImGui::BeginPopupModal("Name New Model"))
            {
                ImGui::InputText("New Name", &m_new_block_model_name_edit);

                if (ImGui::Button("Confirm"))
                {
                    // Some ugly code to copy the current block model and set the current block type's model to the new one
                    const u32 new_model_id = voxel_pack->create_block_model(m_new_block_model_name_edit);

                    if (auto edited_block_type = voxel_pack->block_types()[selected_block_id])
                    {
                        if (const auto prev_model = voxel_pack->get_uncooked_block_model(edited_block_type->model_id))
                        {
                            prev_model->for_each_face(
                                [&](h2o::UncookedBlockModel::FaceHandle, const h2o::UncookedBlockModel::Face& face)
                                {
                                    voxel_pack->add_face_to_model(new_model_id, face);
                                }
                            );
                        }

                        edited_block_type->model_id = new_model_id;
                        voxel_pack->edit_block_type(selected_block_id, *edited_block_type);
                        voxel_pack->build_block_model(new_model_id);
                    }

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