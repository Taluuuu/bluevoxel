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
    static size_t to_index(const v3i& local_pos)
    {
        return
            local_pos.y * voxel_constants::chunk_area +
            local_pos.x * voxel_constants::chunk_size +
            local_pos.z;
    }

    static constexpr bool is_valid_pos(const v3i& local_pos)
    {
        return
            local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
            local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
            local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
    }

    static v3i to_block_pos(size_t index)
    {
        v3i local_pos;

        local_pos.y = index / voxel_constants::chunk_area;
        index %= voxel_constants::chunk_area;

        local_pos.x = index / voxel_constants::chunk_size;
        local_pos.z = index % voxel_constants::chunk_size;

        return local_pos;
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

    u8 Chunk::get_light_level_at(const v3i& local_pos) const
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        return m_light_levels[to_index(local_pos)];
    }

    void Chunk::set_light_level_at(const v3i& local_pos, u8 light_level)
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        m_light_levels[to_index(local_pos)] = light_level;
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

        m_light_levels.resize(voxel_constants::chunk_volume, 0);
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

    void Chunk::update_lighting()
    {
        std::ranges::fill(m_light_levels, 0);

        std::stack<v3i> light_stack{};
        for (i32 i = 0; i < voxel_constants::chunk_size; i++)
        for (i32 j = 0; j < voxel_constants::chunk_size; j++)
        for (i32 k = 0; k < voxel_constants::chunk_size; k++)
        {
            const v3i block_pos{ i, j, k };
            if (get_block_at(block_pos) == 5)
            {
                set_light_level_at(block_pos, voxel_constants::max_light_level);
                light_stack.emplace(block_pos);
            }
        }

        while (!light_stack.empty())
        {
            const v3i lit_block_pos = light_stack.top();
            light_stack.pop();

            static constexpr std::array offsets{
                v3i{-1, 0, 0 },
                v3i{ 1, 0, 0 },
                v3i{ 0,-1, 0 },
                v3i{ 0, 1, 0 },
                v3i{ 0, 0,-1 },
                v3i{ 0, 0, 1 },
            };

            const u8 light_level = get_light_level_at(lit_block_pos);
            for (const v3i& offset : offsets)
            {
                const v3i adj_block_pos = lit_block_pos + offset;
                if (!is_valid_pos(adj_block_pos))
                    continue;

                if (get_block_at(adj_block_pos) != Block::Air)
                    continue;

                const u8 adj_light_level = get_light_level_at(adj_block_pos);
                if (adj_light_level < light_level && light_level > 1)
                {
                    set_light_level_at(adj_block_pos, light_level - 1);
                    light_stack.emplace(adj_block_pos);
                }
            }
        }
    }
}
