#pragma once

#include "core/events.h"
#include "core/types.h"
#include "scene/scene_system.h"
#include "chunk_mesh.h" // Potentially heavy include, made so chunk meshes are contiguous in memory

#include <glm/gtx/hash.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace h2o
{
    // Forward declarations
    class ChunkRenderingRegion;
    class VoxelRenderingModule;
    namespace gfx { class IRenderer; }

    class ChunkRenderingSystem : public SceneSystem
    {
    public:

        explicit ChunkRenderingSystem(const SceneSystemInitializer& system_initializer);

        void update(f32 delta_time) override;
        void render(f32 delta_time) override;

    private:

        VoxelRenderingModule* m_voxel_rendering_module = nullptr;
        gfx::IRenderer* m_renderer = nullptr;

        std::unique_ptr<ChunkRenderingRegion> m_chunk_rendering_region { nullptr };

        v2i m_last_player_chunk {0, 0};

        EventHandle m_on_chunk_created_handle;
        EventHandle m_on_chunk_deleted_handle;
        EventHandle m_on_chunk_updated_handle;
        EventHandle m_on_player_changed_chunk_handle;

    };
}