#include "block_model_editor.h"

#include "block_editor_workspace.h"
#include "core/engine.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

namespace bluevoxel
{
    BlockModelEditor::BlockModelEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_workspace(&workspace)
        , m_ui_module(&g_engine->get_module_checked<h2o::UIModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        set_tick_phases(h2o::TickPhase::Update);
    }

    void BlockModelEditor::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return;

        // TODO: Make easy function to get the block type in the workspace
        const auto& block_type = voxel_pack->block_types()[m_workspace->selected_block_id()];
        if (!block_type)
            return;

        const auto model_id = block_type->model_id;
        const auto& block_model = voxel_pack->block_models()[model_id];

        m_ui_module->window("Block Model Editor", { { 1175.0f, 150.0f }, { 400.0f, 725.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 1);
                ui.label(fmt::format("Editing Model '{}'", block_model.name));

                ui.row(25.0f, 2);
                ui.label("Faces: ");
                if (ui.button("+"))
                    h2o::log::info("CREATE FACE");

            }
        );
    }
}