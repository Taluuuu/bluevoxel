#pragma once

#include "core/events.h"
#include "scene/scene_system.h"

#include <memory>

namespace h2o
{
    namespace gfx
    {
        class IPipeline;
    }

    class VoxelRenderingModule;

    class ChunkRenderingSystem : public SceneSystem
    {
    public:

        explicit ChunkRenderingSystem(const SceneSystemInitializer& system_initializer);

        // SceneSystem interface
        bool init() override;

        void render(f32 delta_time) override;

    private:

        VoxelRenderingModule* const m_voxel_rendering_module = nullptr;

        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;

        EventHandle m_on_chunk_loaded_handle;
        EventHandle m_on_chunk_updated_handle;

    };
}