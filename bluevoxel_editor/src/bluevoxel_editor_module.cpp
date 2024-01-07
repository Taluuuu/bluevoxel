#include "bluevoxel_editor_module.h"

#include "core/engine.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/chunk.h"
#include "voxel/voxel_module.h"
#include "voxel_rendering/voxel_rendering_module.h"

namespace bluevoxel
{
    BlueVoxelEditorModule::BlueVoxelEditorModule()
        : h2o::Tickable(g_engine)
    {}

    bool BlueVoxelEditorModule::init(h2o::Engine& engine)
    {
        m_ui_module = &engine.get_module_checked<h2o::UIModule>();

        auto& voxel_module = engine.get_module_checked<h2o::VoxelModule>();
        h2o::Chunk chunk({ 0, 0, 0 }, voxel_module);

        set_tick_phases(h2o::TickPhase::Update | h2o::TickPhase::Render);
        return true;
    }

    std::vector<std::type_index> BlueVoxelEditorModule::dependencies() const
    {
        return {
            typeid(h2o::VoxelModule),
            typeid(h2o::VoxelRenderingModule),
            typeid(h2o::UIModule) };
    }

    void BlueVoxelEditorModule::update(f32 delta_time)
    {
        m_ui_module->window("Editor", { { 50.0f, 50.0f }, { 250.0f, 250.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 1);

                if (ui.button("TEST"))
                    h2o::log::info("oui");
            }
        );
    }

    void BlueVoxelEditorModule::render()
    {

    }
}