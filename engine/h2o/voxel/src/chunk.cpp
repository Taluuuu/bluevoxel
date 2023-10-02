#include "voxel/chunk.h"

#include "voxel/chunk_system.h"
#include "voxel/voxel_constants.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    static size_t to_index(const v3i& local_pos)
    {
        return
            local_pos.x * voxel_constants::chunk_area +
            local_pos.z * voxel_constants::chunk_size +
            local_pos.y;
    }

    static v3i to_local_block_pos(i32 block_idx)
    {
        return {
            block_idx / voxel_constants::chunk_area,
            block_idx % voxel_constants::chunk_size,
            (block_idx / voxel_constants::chunk_size) % voxel_constants::chunk_size
        };
    }

    void Chunk::init(const VoxelModule& voxel_module)
    {
        m_blocks.resize(voxel_constants::chunk_volume, Block::Air);
        m_voxel_module = &voxel_module;
    }

    Block Chunk::get_block_at(const v3i& local_pos) const
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        return m_blocks[to_index(local_pos)];
    }

    void Chunk::set_block_at(const v3i& local_pos, Block block)
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        set_block_at(to_index(local_pos), block);
    }

    void Chunk::tick()
    {
        for (const u32 block_idx : m_blocks_to_tick)
        {
            Block& block = m_blocks[block_idx];
            auto block_preset = m_voxel_module->get_block_preset(block.id);
            assert(block_preset);

            block_preset->tick(
                block, *this, to_local_block_pos(i32(block_idx)));
        }
    }

    CompressedChunk Chunk::compress() const
    {
        assert(!m_blocks.empty());

        CompressedChunk result{};
        result.chunk_pos = chunk_pos();

        CompressedChunk::BlockCountPair current_pair { m_blocks[0], 0 };

        for (size_t i = 1; i < voxel_constants::chunk_volume; i++)
        {
            if (current_pair.block == m_blocks[i])
            {
                current_pair.count++;
            }
            else
            {
                result.blocks.push_back(current_pair);
                current_pair = { m_blocks[i], 0 };
            }
        }

        result.blocks.push_back(current_pair);

        return result;
    }

    void Chunk::decompress(const CompressedChunk& compressed_chunk)
    {
        assert(!m_blocks.empty());
        size_t compressed_idx = 0;

        m_chunk_pos = compressed_chunk.chunk_pos;

        auto get_next_block_count_pair =
            [&compressed_idx, &compressed_chunk]() -> const CompressedChunk::BlockCountPair*
            {
                if (compressed_idx < compressed_chunk.blocks.size())
                    return &compressed_chunk.blocks[compressed_idx++];

                return nullptr;
            };

        std::pair<Block, u32> current_pair { Block::Air, 0 };

        for (size_t i = 0; i < voxel_constants::chunk_volume; i++)
        {
            while (current_pair.second == 0)
            {
                auto block_count_pair = get_next_block_count_pair();

                if (!block_count_pair)
                    return; // Failure

                current_pair = {
                    block_count_pair->block,
                    block_count_pair->count + 1 };
            }

            set_block_at(i, current_pair.first);
            current_pair.second--;
        }
    }

    void Chunk::set_block_at(size_t index, Block block)
    {
        assert(index < m_blocks.size());

        m_blocks[index] = block;

        // TODO: Check if the block is valid

        if (block != Block::Air)
            m_is_empty = false;

        if (m_voxel_module->get_block_preset_data(block.id).should_tick)
        {
            m_blocks_to_tick.insert(index);
        }
        else
        {
            m_blocks_to_tick.erase(index);
        }
    }
}