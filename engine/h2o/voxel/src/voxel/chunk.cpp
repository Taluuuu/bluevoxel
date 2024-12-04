#include "voxel/chunk.h"

#include "core/engine.h"
#include "voxel/structures/voxel_structure_manager.h"
#include "voxel/voxel_constants.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel/voxel_utils.h"

#include <stack>

namespace h2o
{
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

    void Chunk::place_structure(const VoxelStructureInstance& structure_instance)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        assert(voxel_pack);

        const auto& structure_mgr = voxel_pack->structure_manager();

        const auto& [structure_id, structure_min] = structure_instance;
        const auto structure = structure_mgr.get_structure(structure_id);
        if (!structure)
            return;

        const v3i structure_size = structure->size();

        const v3i chunk_min = m_chunk_pos * voxel_constants::chunk_size;

        const v3i min = glm::max(structure_min, chunk_min);
        const v3i max = glm::min(
            structure_min + structure_size,
            chunk_min + v3i{ voxel_constants::chunk_size });

        voxel_utils::for_v3i(min, max,
            [&](const v3i& block_pos)
            {
                const v3i pos_in_chunk = block_pos - chunk_min;
                const v3i pos_in_structure = block_pos - structure_min;

                if (const auto block = structure->get_block(pos_in_structure); block != Block::Air)
                    set_block_at(pos_in_chunk, block);
            }
        );
    }

    void Chunk::init(const v3i& position)
    {
        m_chunk_pos = position;
        m_voxel_module = &g_engine->get_module_checked<VoxelModule>();

        m_blocks.resize(voxel_constants::chunk_volume, Block::Air);
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

    Block Chunk::get_block_at(const size_t index) const
    {
        return m_blocks[index];
    }

    void Chunk::set_block_at(const size_t index, const Block block)
    {
        assert(index < m_blocks.size());

        m_blocks[index] = block;

        // TODO: Check if the block is valid

        if (block != Block::Air)
            m_is_empty = false;
    }
}
