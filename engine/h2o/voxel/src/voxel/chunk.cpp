#include "voxel/chunk.h"

#include "voxel/voxel_constants.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    static size_t to_index(const v3i& local_pos)
    {
        return
            local_pos.y * voxel_constants::chunk_area +
            local_pos.x * voxel_constants::chunk_size +
            local_pos.z;
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

    void Chunk::init(const v3i& chunk_pos, const VoxelModule& voxel_module)
    {
        m_chunk_pos = chunk_pos;
        m_voxel_module = &voxel_module;

        m_blocks.resize(voxel_constants::chunk_volume, Block::Air);

        m_blocks[0] = { 1 };
    }

    void Chunk::tick()
    {
        // for (const u32 block_idx : m_blocks_to_tick)
        // {
        //     Block& block = m_blocks[block_idx];
        //     if (const auto block_preset = m_voxel_module->get_block_preset(block.id))
        //     {
        //         block_preset->tick(
        //         block, *this, to_local_block_pos(i32(block_idx)));
        //     }
        // }
    }

    Block Chunk::get_block_at(size_t index) const
    {
        return m_blocks[index];
    }

    void Chunk::set_block_at(size_t index, Block block)
    {
        assert(index < m_blocks.size());

//        const Block previous_block = m_blocks[index];
        m_blocks[index] = block;

        // TODO: Check if the block is valid

        if (block != Block::Air)
            m_is_empty = false;
        // m_is_empty = m_is_empty || (block != Block::Air);

//        if (m_voxel_module->get_block_preset_data(block.id).value_or(BlockPresetFlags{}).should_tick)
//        {
//            m_blocks_to_tick.insert(index);
//        }
//        else
//        {
//            m_blocks_to_tick.erase(index);
//        }
    }
}