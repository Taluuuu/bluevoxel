#include "bluevoxel_editor_module.h"

#include "core/engine.h"
#include "game_framework/game_framework_module.h"
#include "input/input_module.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "scene/scene.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/chunk.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_rendering/chunk_mesh_pool.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "world_gen_editor/world_gen_editor_workspace.h"

#include <imgui.h>

namespace bluevoxel
{
    BlueVoxelEditorModule::BlueVoxelEditorModule()
        : h2o::Tickable(g_engine)
    {}

    bool BlueVoxelEditorModule::init(h2o::Engine& engine)
    {
        m_voxel_module = &engine.get_module_checked<h2o::VoxelModule>();
        m_input_module = &engine.get_module_checked<h2o::InputModule>();

        auto& renderer = engine.get_module_checked<h2o::RenderingModule>().renderer();
        renderer.set_clear_color({ 0.16f, 0.26f, 0.34f, 1.0f });

        // TODO: Voxel pack picker in this class ?
        auto voxel_pack = engine
            .resource_mgr()
            .fetch<h2o::VoxelPack>("../Resources/bluevoxel/voxel/");

        if (!voxel_pack)
            return false;

        m_voxel_module->set_voxel_pack(voxel_pack);

        // Input setup
        m_input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        m_input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);
        m_input_module->register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        m_input_module->register_axis("cam_x", h2o::MouseMoveDelta::Y, 0.2f, true);
        m_input_module->register_axis("cam_y", h2o::MouseMoveDelta::X, 0.2f, false);
        m_input_module->register_axis("cam_zoom", h2o::MouseScrollDelta::Y, 1.0f, false);

        set_tick_phases(h2o::TickPhase::Update);

        return true;
    }

    std::vector<std::type_index> BlueVoxelEditorModule::dependencies() const
    {
        return {
            typeid(h2o::UIModule),
            typeid(h2o::GameFrameworkModule),
            typeid(h2o::VoxelModule),
            typeid(h2o::VoxelRenderingModule) };
    }

    void BlueVoxelEditorModule::update(f32 delta_time)
    {
        if (holds_alternative<nullptr_t>(m_editor_mode))
        {
            ImGui::Begin("Editor Selection");

            if (ImGui::Button("Block Editor"))
                m_editor_mode.emplace<BlockEditorWorkspace>(this);

            if (ImGui::Button("World Gen Editor"))
                m_editor_mode.emplace<WorldGenEditorWorkspace>(this);

            ImGui::End();
        }
    }

    void BlueVoxelEditorModule::render()
    {

    }
}