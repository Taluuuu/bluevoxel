#pragma once

#include "core/tickable.h"
#include "voxel/chunk_manager.h"
#include "voxel_client/block_placeable_interface.h"
#include "voxel_rendering/voxel_world_renderer.h"

namespace h2o
{
    class Scene;
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

        [[nodiscard]] v3i calc_extents() const;

    private:

        h2o::ChunkManager m_chunk_manager{};
        h2o::VoxelWorldRenderer m_voxel_world_renderer;

        // The structure's extents
        v3i m_extents{};

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        h2o::EventHandle m_on_chunks_updated_handle{};

        h2o::RenderingModule* m_rendering_module = nullptr;

    };
}
