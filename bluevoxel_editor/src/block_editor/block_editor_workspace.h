#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "selection_manager.h"

#include <memory>

namespace h2o
{
    class Scene;

    class RenderingModule;
    class UIModule;
    class VoxelModule;
}

namespace bluevoxel
{
    class BlockRenderer;
    class BlockTypeEditor;
    class BlockModelEditor;

    class BlockEditorWorkspace : public h2o::Tickable
    {
    public:

        explicit BlockEditorWorkspace(h2o::Tickable* owner);
        ~BlockEditorWorkspace() override = default;

        [[nodiscard]] u32 selected_block_id() const { return m_selected_block_id; }
        void select_block(u32 block_id);

        [[nodiscard]] SelectionManager& selection_mgr() { return m_selection_manager; }

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;
        void render() override;

        void on_voxel_pack_changed();
        void on_voxel_pack_updated();

        void reload_voxel_pack();

    private:

        u32 m_selected_block_id = 0;

        SelectionManager m_selection_manager;

        // Editors
        std::shared_ptr<BlockTypeEditor> m_block_type_editor = nullptr;
        std::shared_ptr<BlockModelEditor> m_block_model_editor = nullptr;

        // Scene
        std::shared_ptr<h2o::Scene> m_scene = nullptr;
        std::shared_ptr<BlockRenderer> m_block_renderer = nullptr;

        // Module refs
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::VoxelModule* const m_voxel_module = nullptr;

        // Event handles
        h2o::EventHandle m_on_voxel_pack_changed_event_handle{};
        h2o::EventHandle m_on_voxel_pack_updated_event_handle{};

    };
}