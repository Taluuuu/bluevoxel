#pragma once

#include "block.h"
#include "core/handle_types.h"
#include "core/types.h"
#include "voxel/direction.h"
#include "voxel/voxel_constants.h"
#include "voxel/voxel_module.h"

#include <iterator>
#include <set>
#include <shared_mutex>
#include <vector>

namespace h2o
{
    class VoxelModule;

    struct CompressedChunk
    {
        struct BlockCountPair
        {
            Block block = Block::Air;

            static_assert(voxel_constants::chunk_volume <= 32768);
            u16 count = 0;
        };

        std::vector<BlockCountPair> blocks;
        v3i chunk_pos;

        template<typename S>
        void serialize(S& s)
        {
            s.container(blocks, voxel_constants::chunk_volume,
                [](S& s, BlockCountPair& block_count_pair)
                {
                    s(block_count_pair.block);
                    s(block_count_pair.count);
                }
            );

            s(chunk_pos);
        }
    };

    class Chunk
    {
    public:

        Chunk(const v3i& chunk_pos, const VoxelModule& voxel_module);

        void tick();

        [[nodiscard]] Block get_block_at(const v3i& local_pos) const;
        void set_block_at(const v3i& local_pos, Block block);

        [[nodiscard]] voxel::Direction::Type get_adjacent_blocks(const v3i& local_pos) const;

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] bool is_empty() const { return m_is_empty; }

        static constexpr bool is_valid_pos(const v3i& local_pos)
        {
            return
                local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
                local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
                local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
        }

        [[nodiscard]] CompressedChunk compress() const;
        void decompress(const CompressedChunk& compressed_chunk);

        [[nodiscard]] std::shared_mutex& mutex() const { return m_mutex; }

    private:

        void set_block_at(size_t index, Block block);
        void set_block_at(size_t index, const v3i& local_pos, Block block);

        [[nodiscard]] bool is_initialized() const { return !m_blocks.empty(); }

    private:

        friend class ChunkColumn;

        std::vector<Block> m_blocks{};
        std::vector<voxel::Direction::Type> m_adjacent_blocks{};

        // Stores the indices of blocks to tick
        std::set<u32> m_blocks_to_tick{};

        const v3i m_chunk_pos{};

        mutable std::shared_mutex m_mutex;

        const VoxelModule* const m_voxel_module = nullptr;

        bool m_is_empty = true;

    };
}