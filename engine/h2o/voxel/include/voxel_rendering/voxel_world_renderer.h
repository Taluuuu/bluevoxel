#pragma once

#include "chunk_mesh_pool.h"
#include "core/events.h"
#include "core/tickable.h"
#include "voxel/chunk_manager.h"

namespace h2o
{
    class RenderingModule;
    class VoxelModule;

    enum class ChunkRenderMode
    {
        DrawAllChunks,
        DrawChunksWithAdjacentChunks
    };

    class VoxelWorldRenderer : public Tickable
    {
    public:

        VoxelWorldRenderer(
            Tickable& owner,
            ChunkManager& chunk_manager,
            ChunkRenderMode chunk_render_mode = ChunkRenderMode::DrawChunksWithAdjacentChunks);

    public:

        // TODO: Create a real engine-wide lighting system
        v3 light_dir { 0.18f, -1.0f, 0.492f };
        v3 light_color { 1.0f, 1.0f, 1.0f };
        f32 ambient_strength = 0.714f;

        v3 player_pos{};

    protected:

        // Tickable interface
        void update(f32 delta_time) override;
        void pre_render() override;
        void render() override;

        void queue_chunk_remesh(const v3i& chunk_pos);
        void update_chunk_lighting(const ChunkView& chunk_view);
        void remesh_chunk_immediate(const v3i& chunk_pos);

    private:

        ChunkMeshPool m_chunk_mesh_pool{};

        std::mutex m_chunks_pending_remesh_mutex{};
        std::unordered_set<v3i> m_chunks_pending_remesh{};

        std::mutex m_pending_built_meshes_mutex{};
        std::vector<ChunkMesh> m_pending_built_meshes{};

        ChunkRenderMode m_render_mode = ChunkRenderMode::DrawChunksWithAdjacentChunks;

        EventHandle m_on_chunk_updated_handle{};
        EventHandle m_on_chunk_deleted_handle{};

        ChunkManager* m_chunk_manager = nullptr;
        const RenderingModule* m_rendering_module = nullptr;
        const VoxelModule* m_voxel_module = nullptr;

    };
}
