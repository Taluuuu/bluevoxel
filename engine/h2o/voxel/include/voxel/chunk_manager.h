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
            const std::function<void(const View<CellType>&)>& function);

        [[nodiscard]] bool is_chunk_column_generated(v2i chunk_column_pos);

    private:

        EventHandle m_chunks_updated_handle{};

    };

    namespace voxel
    {
        [[nodiscard]] std::optional<Block> get_block_at(
            const View<Chunk>& view,
            const v3i& block_pos,
            EViewRelativeTo relative_to = EViewRelativeTo::World);

        bool set_block_at(
            View<Chunk>& view,
            const v3i& block_pos,
            Block block,
            EViewRelativeTo relative_to = EViewRelativeTo::World);

        // Loop through all blocks that are not air
        void for_each_block(
            const View<Chunk>& view,
            const std::function<void(const v3i&, const Block&)>& function);

        [[nodiscard]] bool is_generated(const View<Chunk>& view);

    }

    namespace chunk_lighting
    {
        void update_lighting(
            View<ChunkLighting>& lighting_view,
            const View<Chunk>& chunk_view);

        void set_light_level(
            View<ChunkLighting>& view,
            const v3i& block_pos, u8 light_level,
            EViewRelativeTo relative_to = EViewRelativeTo::World);

        [[nodiscard]] u8 get_light_level(
            View<ChunkLighting>& view,
            const v3i& block_pos,
            EViewRelativeTo relative_to = EViewRelativeTo::World);
    }

    template<class CellType>
    void ChunkManager::view_for_meshing(
        const v3i& chunk_pos,
        const std::function<void(const View<CellType>&)>& function)
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
