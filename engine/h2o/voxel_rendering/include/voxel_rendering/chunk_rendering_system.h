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

        void render(f32 delta_time) override;

    private:

//        void create_mesh_at(gfx::IRenderer& renderer, const v3i& chunk_pos);
//        void erase_mesh_at(const v3i& chunk_pos);

    private:

        VoxelRenderingModule* m_voxel_rendering_module = nullptr;
        gfx::IRenderer* m_renderer = nullptr;

        std::unique_ptr<ChunkRenderingRegion> m_chunk_rendering_region { nullptr };

        EventHandle m_on_chunk_created_handle;
        EventHandle m_on_chunk_deleted_handle;
        EventHandle m_on_chunk_updated_handle;
        EventHandle m_on_player_changed_chunk_handle;

    };
}