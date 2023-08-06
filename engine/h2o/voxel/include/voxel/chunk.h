#pragma once

#include "block.h"
#include "core/types.h"
#include "voxel/voxel_constants.h"

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

        void init(const v3i& chunk_pos);

        [[nodiscard]] Block get_block_at(const v3i& local_pos) const;
        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] bool is_empty() const { return m_is_empty; }
        void set_block_at(const v3i& local_pos, Block block);

        static constexpr bool is_valid_pos(const v3i& local_pos)
        {
            return
                local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
                local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
                local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
        }

    private:

        std::vector<Block> m_blocks;

        v3i m_chunk_pos{};

        bool m_is_empty = true;

    };
}