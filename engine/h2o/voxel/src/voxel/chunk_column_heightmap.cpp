#include "voxel/chunk_column_heightmap.h"

#include "voxel/chunk.h"

namespace h2o
{
    static constexpr size_t to_index(const v2i block_pos)
    {
        assert(
            block_pos.x >= 0 && block_pos.x < voxel_constants::chunk_size &&
            block_pos.y >= 0 && block_pos.y < voxel_constants::chunk_size);

        return block_pos.x * voxel_constants::chunk_size + block_pos.y;
    }

    void ChunkColumnHeightmap::update(const Chunk& chunk, const v3i& block_pos)
    {
        const i32 chunk_y = chunk.chunk_pos().y;
        assert(chunk_y >= 0 && chunk_y < voxel_constants::vertical_chunk_count);

        auto& chunk_heightmap = m_chunk_heightmaps[chunk_y];

        const size_t height_index = to_index({ block_pos.x, block_pos.z });
        u8& height_in_chunk = chunk_heightmap[height_index];

        i32 y = glm::max(block_pos.y, i32(height_in_chunk) - 1);
        height_in_chunk = 0;
        for (; y >= 0; y--)
        {
            // NOTE: This could probably be made faster by using indexed access and subtracting
            //       the offset between block positions that are vertically aligned
            const Block block = chunk.get_block_at({ block_pos.x, y, block_pos.z });
            if (block != Block::Air)
            {
                height_in_chunk = y + 1;
                break;
            }
        }

        // Update m_column_heightmap
        for (i32 i = voxel_constants::vertical_chunk_count - 1; i >= 0; i--)
        {
            const auto& temp = m_chunk_heightmaps[i];
            if (const u8 height = temp[height_index]; height > 0)
            {
                m_column_heightmap[height_index] = voxel_constants::chunk_size * i + u32(height);
                break;
            }
        }
    }

    u32 ChunkColumnHeightmap::get_height(const v2i block_pos) const
    {
        return m_column_heightmap[to_index(block_pos)];
    }
}
