#pragma once

#include "chunk.h"
#include "chunk_lighting.h"
#include "grid/grid_3d.h"

#include <atomic>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace h2o
{
    struct ChunksUpdatedEvent { const std::unordered_set<v3i>& updated_chunks{}; };
    struct ChunksDeletedEvent { const std::unordered_set<v2i>& deleted_chunk_columns{}; };

    enum class ChunkRenderMode
    {
        DrawAllChunks,
        RequireAdjacentChunks
    };

    class ChunkManager : public Grid3D<voxel_constants::vertical_chunk_count, Chunk, ChunkLighting>
    {
    public:

        ChunkManager(ChunkRenderMode chunk_render_mode);

        [[nodiscard]] Block get_block_at(const v3i& block_pos);
        bool set_block_at(const v3i& block_pos, Block block);

        template<class CellType>
        void view_for_meshing(
            const v3i& chunk_pos,
            const std::function<void(const ViewType<CellType>&)>& function) const;

        void set_player_positions(const std::vector<v3>& player_positions);

        [[nodiscard]] bool is_chunk_column_generated(v2i chunk_column_pos) const;
        [[nodiscard]] bool is_ready_for_meshing(const v3i& chunk_pos) const;
        [[nodiscard]] bool is_ready_for_lighting_update(const v3i& chunk_pos) const;
        [[nodiscard]] bool is_pending_lighting_update(const v3i& chunk_pos) const;

        void broadcast_events() override;

    protected:

        // Grid3D interface
        std::shared_ptr<CellColumnTuple> create_cell_column(v2i cell_column_pos) override;

        void request_lighting_update(std::unordered_set<v3i> chunks_to_update_lighting, f32 priority_override = -1.0f);

    private:

        void on_chunks_updated(const CellsUpdatedEvent& event);
        void on_chunks_deleted(const CellsDeletedEvent& event);

    private:

        // Chunk lightings that are waiting to be updated
        mutable std::shared_mutex m_chunk_positions_pending_lighting_update_mutex{};
        std::unordered_set<v3i> m_chunk_positions_pending_lighting_update{};

        // Chunk lightings that are waiting to be broadcast as updated
        mutable std::shared_mutex m_chunk_positions_after_lighting_update_mutex{};
        std::unordered_set<v3i> m_chunk_positions_after_lighting_update{};

        // Chunk lightings that were updated at least one
        mutable std::shared_mutex m_built_chunk_lightings_mutex{};
        std::unordered_set<v3i> m_built_chunk_lightings{};

        // Chunks that were updated at least one
        mutable std::shared_mutex m_generated_chunks_mutex{};
        std::unordered_set<v3i> m_generated_chunks{};

        mutable std::shared_mutex m_player_positions_mutex{};
        std::vector<v3> m_player_positions{};

        EventHandle m_chunks_updated_handle{};
        EventHandle m_chunks_deleted_handle{};

        ChunkRenderMode m_chunk_render_mode = ChunkRenderMode::DrawAllChunks;

    };

    namespace chunk_lighting
    {
        void update_lighting(ChunkLighting& chunk_lighting, const Chunk::ViewType& chunk_view);

        void propagate_lighting(
            ChunkLighting::ViewType& lighting_view,
            const Chunk::ViewType& chunk_view,
            ChunkLightingType lighting_type,
            const std::vector<v3i>& light_sources);
    }

    template<class CellType>
    void ChunkManager::view_for_meshing(
        const v3i& chunk_pos,
        const std::function<void(const ViewType<CellType>&)>& function) const
    {
        view_impl<CellType>(chunk_pos - v3i{1}, v3i{3},
            [&](const v3i& chunk_pos_to_check) -> bool
            {
                // Only allow directly adjacent
                const v3i offset = glm::abs(chunk_pos_to_check - chunk_pos);
                return (offset.x + offset.y + offset.z) <= 1;
            }, function
        );
    }
}
