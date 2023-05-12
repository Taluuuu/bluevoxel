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
    class VoxelRenderingModule;

    class ChunkRenderingSystem : public SceneSystem
    {
    public:

        explicit ChunkRenderingSystem(const SceneSystemInitializer& system_initializer);

        // SceneSystem interface
        bool init() override;

        void render(f32 delta_time) override;

    private:

        // Map for fast retrieval of chunk meshes by their world position
        std::unordered_map<v3i, size_t> m_mesh_index_map;
        // Vector for fast traversal
        std::vector<ChunkMesh> m_chunk_meshes;

        VoxelRenderingModule* m_voxel_rendering_module = nullptr;

        EventHandle m_on_chunk_created_handle;
        EventHandle m_on_chunk_deleted_handle;
        EventHandle m_on_chunk_updated_handle;

    };
}