#pragma once

#include "block.h"
#include "voxel_constants.h"

namespace h2o
{
    class Chunk;

    class CompressedChunk
    {
    public:

        CompressedChunk() = default;
        explicit CompressedChunk(const Chunk& chunk);

        bool decompress(Chunk& dest) const;
        [[nodiscard]] v3i chunk_pos() const { return m_chunk_pos; }

        template<typename S>
        void serialize(S& s)
        {
            s.container(m_blocks, voxel_constants::chunk_volume,
                [](S& s, BlockCountPair& block_count_pair)
                {
                    s(block_count_pair.block);
                    s(block_count_pair.count);
                }
            );

            s(m_chunk_pos);
        }

        struct BlockCountPair
        {
            Block block = Block::Air;

            static_assert(voxel_constants::chunk_volume <= 32768);
            u16 count = 0;
        };

    private:

        std::vector<BlockCountPair> m_blocks{};
        v3i m_chunk_pos{};

    };
}