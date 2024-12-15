#pragma once

#include "chunk.h"
#include "chunk_lighting.h"
#include "grid/grid_3d.h"

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

    class ChunkManager : public Grid3D<voxel_constants::vertical_chunk_count, Chunk, ChunkLighting>
    {
    public:

        ChunkManager();

        [[nodiscard]] Block get_block_at(const v3i& block_pos);
        bool set_block_at(const v3i& block_pos, Block block);

        template<class CellType>
        void view_for_meshing(
            const v3i& chunk_pos,
            const std::function<void(const ViewType<CellType>&)>& function);

        [[nodiscard]] bool is_chunk_column_generated(v2i chunk_column_pos);

        void broadcast_events() override;

    private:

        void on_chunks_updated(const CellsUpdatedEvent& event);

    private:

        std::mutex m_chunk_positions_pending_lighting_update_mutex{};
        std::unordered_set<v3i> m_chunk_positions_pending_lighting_update{};

        std::mutex m_chunk_positions_after_lighting_update_mutex{};
        std::unordered_set<v3i > m_chunk_positions_after_lighting_update{};

        EventHandle m_chunks_updated_handle{};

    };

    namespace chunk_lighting
    {
        void update_lighting(
            ChunkLighting& chunk_lighting,
            const Chunk::ViewType& chunk_view);
    }

    template<class CellType>
    void ChunkManager::view_for_meshing(
        const v3i& chunk_pos,
        const std::function<void(const ViewType<CellType>&)>& function)
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
