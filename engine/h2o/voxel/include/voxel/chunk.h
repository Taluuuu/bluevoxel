#pragma once

#include "core/types.h"
#include "block.h"

#include <cstddef>
#include <iterator>
#include <vector>

namespace h2o
{
    class ChunkSystem;

    class Chunk
    {
    public:

        Chunk() = default;

        void init(const ChunkSystem& chunk_system, const v3i& chunk_pos);

        [[nodiscard]] Block get_block_at(const v3u& local_pos) const;
        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        void set_block_at(const v3u& local_pos, Block block);

    private:

        static constexpr size_t to_index(const v3u& local_pos);
        static constexpr bool is_valid_pos(const v3u& local_pos);

    private:

        std::vector<Block> m_blocks;

        v3i m_chunk_pos{};

        const ChunkSystem* m_chunk_system = nullptr;

    };
}