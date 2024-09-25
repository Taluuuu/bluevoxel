#pragma once

#include "core/tickable.h"
#include "gizmo.h"
#include "selection_manager.h"
#include "voxel/chunk_manager.h"
#include "voxel_client/block_placeable_interface.h"
#include "voxel_rendering/voxel_world_renderer.h"

namespace h2o
{
    class Scene;
    class VoxelStructure;
    class VoxelStructureManager;
}

namespace bluevoxel
{
    class StructureEditorWorkspace
        : public h2o::Tickable
        , public oup::enable_observer_from_this_unique<StructureEditorWorkspace>
        , public h2o::IBlockPlaceable
    {
    public:

        explicit StructureEditorWorkspace(h2o::Tickable* owner);
        ~StructureEditorWorkspace() override = default;

        // h2o::IBlockPlaceable interface
        void set_block_at(const v3i& block_pos, h2o::Block block) override;
        [[nodiscard]] h2o::ChunkManager& chunk_mgr() override { return m_chunk_manager; }
        [[nodiscard]] const h2o::ChunkManager& chunk_mgr() const override { return m_chunk_manager; }

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;
        void render() override;

        void tick_editor_gui();
        void load_selected_structure();
        void save_structure();

        void open_rename_structure_popup();
        void open_delete_structure_popup();

        [[nodiscard]] v3i calc_extents() const;

        [[nodiscard]] static h2o::VoxelPack& get_voxel_pack();
        [[nodiscard]] static h2o::VoxelStructureManager& get_structure_mgr();
        [[nodiscard]] h2o::VoxelStructure* get_selected_structure() const;

    private:

        static constexpr h2o::Block default_block = 3;

        h2o::ChunkManager m_chunk_manager{};
        h2o::VoxelWorldRenderer m_voxel_world_renderer;

        u32 m_selected_structure_id = 0;

        // The structure's extents
        v3i m_extents{};

        // Gizmo for moving a whole structure
        SelectionManager m_selection_mgr;
        Gizmo m_structure_gizmo;

        // Modals
        // The input for the structure currently being renamed.
        std::string m_selected_structure_name_edit{};

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        h2o::EventHandle m_on_chunks_updated_handle{};

        h2o::RenderingModule* m_rendering_module = nullptr;

    };
}
