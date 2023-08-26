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

        // TODO: Check if the block is valid

        const size_t block_idx = to_index(local_pos);
        m_blocks[block_idx] = block;

        if (block == Block::Air)
        {
            // Check if chunk is now empty
            // Whoops, this is really fucking slow right now because of world gen code...
            // This check should really be done after that and each time a block is placed.
//            m_is_empty = true;
//            for (const Block& b : m_blocks)
//            {
//                if (b != Block::Air)
//                {
//                    m_is_empty = false;
//                    break;
//                }
//            }
        }
        else
        {
            m_is_empty = false;
        }

        if (m_voxel_module->get_block_preset_data(block.id).should_tick)
        {
            m_blocks_to_tick.insert(block_idx);
        }
        else
        {
            m_blocks_to_tick.erase(block_idx);
        }
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
}