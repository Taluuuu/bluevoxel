#pragma once

#include "core/types.h"
#include "block.h"

#include <cstddef>
#include <iterator>
#include <vector>

namespace h2o
{
    class ChunkMgr;

    class Chunk
    {
    public:

        explicit Chunk(const ChunkMgr& chunk_mgr);

        [[nodiscard]] Block get_block_at(const v3u& local_pos) const;
        void set_block_at(const v3u& local_pos, Block block);

    private:

        static constexpr size_t to_index(const v3u& local_pos);
        static constexpr bool is_valid_pos(const v3u& local_pos);

    private:

        std::vector<Block> m_blocks;

        const ChunkMgr* const m_chunk_mgr = nullptr;

    };
}