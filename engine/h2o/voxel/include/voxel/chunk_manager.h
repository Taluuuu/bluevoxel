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

        [[nodiscard]] Block get_block_at(const v3i& block_pos);
        bool set_block_at(const v3i& block_pos, Block block);

        void view_for_meshing(
            const v3i& chunk_pos,
            const std::function<void(const View<const Chunk>&)>& function);

        [[nodiscard]] bool is_chunk_column_generated(v2i chunk_column_pos);

    };

    // TODO: using CM = ChunkManager ? or
    // template<class CellType>
    // using CMView = ChunkManager::View<CellType>;

    namespace voxel
    {
        [[nodiscard]] std::optional<Block> get_block_at(const ChunkManager::View<const Chunk>& view, const v3i& block_pos, EViewRelativeTo relative_to = EViewRelativeTo::World);
        [[nodiscard]] std::optional<Block> get_block_at(const ChunkManager::View<Chunk>& view, const v3i& block_pos, EViewRelativeTo relative_to = EViewRelativeTo::World);

        bool set_block_at(
            const ChunkManager::View<Chunk>& view,
            const v3i& block_pos,
            Block block,
            EViewRelativeTo relative_to = EViewRelativeTo::World);

        // Loop through all blocks that are not air
        void for_each_block(
            const ChunkManager::View<const Chunk>& view,
            const std::function<void(const v3i&, const Block&)>& function);

        [[nodiscard]] bool is_generated(const ChunkManager::View<const Chunk>& view);

        void update_lighting(
            const ChunkManager::View<ChunkLighting>& lighting_view,
            const ChunkManager::View<const Chunk>& chunk_view);

    }
}
