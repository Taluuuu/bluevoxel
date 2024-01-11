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
        m_selected_block_name_edit = std::nullopt;
    }

    void BlockTypeEditor::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return;

        u32 selected_block_id = m_workspace->selected_block_id();

        m_ui_module->window("Block Type Editor", { { 25.0f, 25.0f }, { 400.0f, 850.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 1);

                if (ui.button("Create Block"))
                {
                    m_workspace->select_block(voxel_pack->create_block_type("new_block"));
                    m_selected_block_name_edit = "new_block";
                }

                ui.row(25.0f, 1);

                // Block type picker
                if (ui.combobox(m_block_type_names_c_str, selected_block_id))
                    m_workspace->select_block(selected_block_id);

                if (auto edited_block_type = voxel_pack->block_types()[selected_block_id]; edited_block_type->block_id != 0)
                {
                    ui.label(fmt::format("id: {}", selected_block_id));

                    ui.row(25.0f, 2);

                    if (ui.button(fmt::format("Delete '{}'", edited_block_type->name)))
                        voxel_pack->delete_block_type(selected_block_id);

                    if (ui.button(fmt::format("Rename '{}'", edited_block_type->name)))
                        m_selected_block_name_edit = edited_block_type->name;

                    ui.row(25.0f, 2);

                    ui.label("Block preset: ");
                    if (ui.combobox(m_block_preset_names_c_str, edited_block_type->preset_id))
                        voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                    ui.label("Block model: ");
                    if (ui.combobox(m_block_model_names_c_str, edited_block_type->model_id))
                        voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                    ui.row(25.0f, 1);

                    ui.label("Textures: ");
                    const auto& model = voxel_pack->block_models()[edited_block_type->model_id];
                    for (u32 i = 0; i < model.calculate_face_count(); i++)
                    {
                        if (ui.combobox(m_texture_names_c_str, edited_block_type->texture_ids[i]))
                            voxel_pack->edit_block_type(selected_block_id, *edited_block_type);
                    }
                }
            }
        );

        if (m_selected_block_name_edit)
        {
            m_ui_module->window("Name Selected Block", { { 700.0f, 25.0f }, { 200.0f, 125.0f } },
                [&](h2o::IUIRenderer& ui)
                {
                    if (!voxel_pack)
                        return;

                    auto edited_block_type = voxel_pack->block_types()[selected_block_id];
                    if (!edited_block_type)
                        return;

                    ui.row(25.0f, 1);

                    ui.input_text("New Name", *m_selected_block_name_edit);

                    ui.row(25.0f, 2);

                    if (ui.button("Confirm"))
                    {
                        edited_block_type->name = *m_selected_block_name_edit;
                        voxel_pack->edit_block_type(selected_block_id, *edited_block_type);

                        m_selected_block_name_edit = std::nullopt;
                    }

                    if (ui.button("Cancel"))
                    {
                        m_selected_block_name_edit = std::nullopt;
                    }
                }
            );
        }
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
            const auto& block_models = voxel_pack->block_models();

            m_block_model_names_c_str.reserve(block_models.size());
            for (const auto& block_model: block_models)
                m_block_model_names_c_str.push_back(block_model.name.c_str());
        }
    }
}