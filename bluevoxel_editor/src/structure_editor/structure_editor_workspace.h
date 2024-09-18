#pragma once

#include "core/tickable.h"
#include "voxel/chunk_manager.h"
#include "voxel_rendering/voxel_world_renderer.h"

namespace h2o
{
    class Scene;
}

namespace bluevoxel
{
    class StructureEditorWorkspace : public h2o::Tickable
    {
    public:

        explicit StructureEditorWorkspace(h2o::Tickable* owner);
        ~StructureEditorWorkspace() override = default;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

    private:

        h2o::ChunkManager m_chunk_manager{};
        h2o::VoxelWorldRenderer m_voxel_world_renderer;

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

    };
}
