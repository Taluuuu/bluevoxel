#include "voxel/compressed_chunk.h"

#include "voxel/chunk.h"

namespace h2o
{
    CompressedChunk::CompressedChunk(const Chunk& chunk)
    {
        m_chunk_pos = chunk.chunk_pos();

        BlockCountPair current_pair{
            chunk.get_block_at({ 0, 0, 0 }), 0 };

        for (size_t i = 1; i < voxel_constants::chunk_volume; i++)
        {
            const auto block = chunk.get_block_at(i);
            if (current_pair.block == block)
            {
                current_pair.count++;
            }
            else
            {
                m_blocks.push_back(current_pair);
                current_pair = { block, 0 };
            }
        }

        m_blocks.push_back(current_pair);
    }

    bool CompressedChunk::decompress(Chunk& dest) const
    {
        size_t compressed_idx = 0;

        const auto get_next_block_count_pair =
            [&]() -> const BlockCountPair*
            {
                if (compressed_idx < m_blocks.size())
                    return &m_blocks[compressed_idx++];

                return nullptr;
            };

        std::pair<Block, u32> current_pair { Block::Air, 0 };

        for (size_t i = 0; i < voxel_constants::chunk_volume; i++)
        {
            while (current_pair.second == 0)
            {
                auto block_count_pair = get_next_block_count_pair();

                if (!block_count_pair)
                    return false; // Failure

                current_pair = {
                    block_count_pair->block,
                    block_count_pair->count + 1 };
            }

            dest.set_block_at(i, current_pair.first);
            current_pair.second--;
        }

        dest.mark_generated();

        return true;
    }
}
