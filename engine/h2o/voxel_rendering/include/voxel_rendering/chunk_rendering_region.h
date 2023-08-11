#pragma once

#include "chunk_mesh.h"
#include "core/distance_queue.h"
#include "core/tickable.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"

#include <array>
#include <memory>

namespace h2o
{
    class ChunkRenderingRegion
        : public ChunkRegion
        , public Tickable
    {
    public:

        ChunkRenderingRegion(
            gfx::IRenderer& renderer,
            const VoxelRenderingModule& voxel_rendering_module,
            ChunkSystem& chunk_system);

        // Tickable interface
        void update(f32 delta_time) override;

        void for_each_chunk_mesh(const std::function<void(ChunkMesh&)>& fun) const;

        [[nodiscard]] i32 num_chunks_pending_mesh_update() const
        { return i32(m_chunks_to_mesh.size()); }

    protected:

        using ChunkMeshColumn = std::array<ChunkMesh, voxel_constants::vertical_chunk_count>;
        // Chunk mesh columns should be easy to move.
        using ChunkMeshColumnOwner = std::unique_ptr<ChunkMeshColumn>;

        // ChunkRegion interface
        void on_indices_changed(const std::vector<i32>& new_to_old_indices) override;
        void on_chunk_fetched(const WeakHandle<ChunkColumn>& chunk_col) override;

    private:

        [[nodiscard]] ChunkMeshColumn* fetch_or_create_chunk_mesh_column(v2i chunk_col_pos);
        [[nodiscard]] ChunkMeshColumn* fetch_chunk_mesh_column(v2i chunk_col_pos) const;
        [[nodiscard]] ChunkMeshColumnOwner create_chunk_mesh_column(v2i chunk_col_pos) const;

        [[nodiscard]] bool fetch_adjacent_chunks(v3i chunk_pos, std::array<Chunk*, 6>& out_adj_chunks) const;

        void remove_out_of_range_chunk_mesh_requests();
        void update_next_chunk_mesh();

    public:

        v3 player_pos{};

    private:

        std::vector<ChunkMeshColumnOwner> m_chunk_mesh_columns;
        DistanceQueue<ChunkWeakHandle> m_chunks_to_mesh;

        EventHandle m_on_chunk_updated_handle;

        gfx::IRenderer* const m_renderer = nullptr;
        const VoxelRenderingModule* const m_voxel_rendering_module = nullptr;

    };
}