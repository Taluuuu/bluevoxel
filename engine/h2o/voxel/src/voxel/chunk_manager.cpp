#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    Block ChunkManager::get_block_at(const v3i& block_pos) const
    {
        Block block = Block::Air;
        fetch<Chunk>(voxel_utils::block_to_chunk_pos(block_pos),
            [&](const Chunk* chunk)
            {
                if (chunk)
                    block = chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
            }
        );

        return block;
    }

    bool ChunkManager::set_block_at(const v3i& block_pos, const Block block)
    {
        bool success = false;
        fetch_mut<Chunk>(voxel_utils::block_to_chunk_pos(block_pos),
            [&](Chunk* chunk)
            {
                if (chunk)
                {
                    chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
                    success = true;
                }
            }
        );

        return success;
    }

    void ChunkManager::view_for_meshing(const v3i& chunk_pos, const std::function<void(const View<Chunk>&)>& function) const
    {
        view_impl<Chunk>(chunk_pos - v3i{1}, v3i{3},
            [&](const v3i& chunk_pos_to_check) -> bool
            {
                // Only allow directly adjacent
                const v3i offset = glm::abs(chunk_pos_to_check - chunk_pos);
                return (offset.x + offset.y + offset.z) <= 1;
            }, function
        );
    }

    std::optional<Block> voxel::get_block_at(
        const ChunkManager::View<Chunk>& view,
        const v3i& block_pos,
        const EViewRelativeTo relative_to)
    {
        if (const Chunk* chunk = view.get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    bool voxel::set_block_at(
        ChunkManager::View<Chunk>& view,
        const v3i& block_pos,
        const Block block,
        const EViewRelativeTo relative_to)
    {
        if (Chunk* chunk = view.get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    void voxel::for_each_block(
        const ChunkManager::View<Chunk>& view,
        const std::function<void(const v3i&, const Block&)>& function)
    {
        view.for_each_cell(
            [&](const Chunk& chunk, const v3i&)
            {
                if (chunk.is_empty())
                    return;

                const v3i chunk_corner_pos = chunk.chunk_pos() * voxel_constants::chunk_size;
                for (i32 i = 0; i < voxel_constants::chunk_size; i++)
                for (i32 j = 0; j < voxel_constants::chunk_size; j++)
                for (i32 k = 0; k < voxel_constants::chunk_size; k++)
                {
                    const v3i local_block_pos{ i, j, k };
                    const v3i world_block_pos = chunk_corner_pos + local_block_pos;
                    if (const auto block = chunk.get_block_at(local_block_pos); block != Block::Air)
                        function(world_block_pos, block);
                }
            }
        );
    }

    bool voxel::is_generated(const ChunkManager::View<Chunk>& view)
    {
        return !view.any_matches([](const Chunk* chunk) { return !chunk || !chunk->is_generated(); });
    }
}
