#pragma once

#include "core/types.h"
#include "voxel_constants.h"

namespace h2o
{
    constexpr i32 non_stupid_mod(i32 x, i32 m)
    {
        i32 r = x % m;
        return r + m * (r < 0);
    }

    constexpr v3i world_to_block_pos(const v3& world_pos)
    {
        return
        {
            static_cast<i32>(world_pos.x) - (world_pos.x < 0.0f),
            static_cast<i32>(world_pos.y) - (world_pos.y < 0.0f),
            static_cast<i32>(world_pos.z) - (world_pos.z < 0.0f)
        };
    }

    constexpr v3i block_to_chunk_pos(const v3i& block_pos)
    {
        return
        {
            (block_pos.x + (block_pos.x < 0)) / voxel_constants::chunk_size - (block_pos.x < 0),
            (block_pos.y + (block_pos.y < 0)) / voxel_constants::chunk_size - (block_pos.y < 0),
            (block_pos.z + (block_pos.z < 0)) / voxel_constants::chunk_size - (block_pos.z < 0)
        };
    }

    constexpr v2i block_to_chunk_pos(const v2i& block_pos)
    {
        return
        {
            (block_pos.x + (block_pos.x < 0)) / voxel_constants::chunk_size - (block_pos.x < 0),
            (block_pos.y + (block_pos.y < 0)) / voxel_constants::chunk_size - (block_pos.y < 0)
        };
    }

    constexpr v3i world_to_chunk_pos(const v3& world_pos)
    {
        return block_to_chunk_pos(world_to_block_pos(world_pos));
    }

    constexpr v3i block_pos_to_within_chunk(const v3i& block_pos)
    {
        return
        {
            non_stupid_mod(block_pos.x, voxel_constants::chunk_size),
            non_stupid_mod(block_pos.y, voxel_constants::chunk_size),
            non_stupid_mod(block_pos.z, voxel_constants::chunk_size)
        };
    }

    constexpr v3 chunk_to_world_pos(const v3i& chunk_pos)
    {
        return chunk_pos * voxel_constants::chunk_size;
    }
}