#pragma once

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

    private:

        VoxelRenderingModule* const m_voxel_rendering_module = nullptr;

        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;

    };
}