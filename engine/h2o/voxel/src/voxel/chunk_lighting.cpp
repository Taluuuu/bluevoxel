#include "voxel/chunk_lighting.h"

#include "core/engine.h"
#include "voxel/chunk.h"

#include <stack>

namespace h2o
{
    ChunkLighting::ChunkLighting()
    {
        m_light_levels.resize(voxel_constants::chunk_volume, 0);
    }

    u8 ChunkLighting::get_light_level(const v3i& local_block_pos) const
    {
        assert(is_valid_pos(local_block_pos));
        return m_light_levels[Chunk::to_index(local_block_pos)];
    }

    void ChunkLighting::set_light_level(const v3i& local_block_pos, const u8 light_level)
    {
        assert(is_valid_pos(local_block_pos));
        m_light_levels[Chunk::to_index(local_block_pos)] = light_level;
    }

    void ChunkLighting::update_lighting(const std::function<Block(const v3i&)>& get_block_function)
    {
        std::ranges::fill(m_light_levels, 0);

        // TODO: Maybe figure some better place for this
        const VoxelModule& voxel_module = g_engine->get_module_checked<VoxelModule>();

        std::stack<v3i> light_stack{};
        for (i32 i = 0; i < voxel_constants::chunk_size; i++)
        for (i32 j = 0; j < voxel_constants::chunk_size; j++)
        for (i32 k = 0; k < voxel_constants::chunk_size; k++)
        {
            const v3i block_pos{ i, j, k };
            if (get_block_function(block_pos) == 5)
            {
                set_light_level(block_pos, voxel_constants::max_light_level);
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

            const u8 light_level = get_light_level(lit_block_pos);
            for (const v3i& offset : offsets)
            {
                const v3i adj_block_pos = lit_block_pos + offset;
                if (!Chunk::is_valid_pos(adj_block_pos))
                    continue;

                const Block block = get_block_function(adj_block_pos);
                if (!voxel_module.is_transparent(block.id))
                    continue;

                const u8 adj_light_level = get_light_level(adj_block_pos);
                if (adj_light_level < light_level && light_level > 1)
                {
                    set_light_level(adj_block_pos, light_level - 1);
                    light_stack.emplace(adj_block_pos);
                }
            }
        }
    }
}
