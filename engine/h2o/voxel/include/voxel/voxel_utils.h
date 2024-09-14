#pragma once

#include "core/types.h"
#include "voxel_constants.h"

namespace h2o::voxel_utils
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

    constexpr v2 chunk_to_world_pos(const v2i& chunk_pos)
    {
        return chunk_pos * voxel_constants::chunk_size;
    }

    constexpr v3 chunk_to_world_pos(const v3i& chunk_pos)
    {
        return chunk_pos * voxel_constants::chunk_size;
    }

    constexpr v2i chunk_to_region_pos(const v2i& chunk_pos)
    {
        return
        {
            (chunk_pos.x + (chunk_pos.x < 0)) / voxel_constants::chunk_region_size - (chunk_pos.x < 0),
            (chunk_pos.y + (chunk_pos.y < 0)) / voxel_constants::chunk_region_size - (chunk_pos.y < 0)
        };
    }

    constexpr v3i chunk_to_region_pos(const v3i& chunk_pos)
    {
        return
        {
            (chunk_pos.x + (chunk_pos.x < 0)) / voxel_constants::chunk_region_size - (chunk_pos.x < 0),
            (chunk_pos.y + (chunk_pos.y < 0)) / voxel_constants::chunk_region_size - (chunk_pos.y < 0),
            (chunk_pos.z + (chunk_pos.z < 0)) / voxel_constants::chunk_region_size - (chunk_pos.z < 0)
        };
    }

    template<typename T = v3i>
    constexpr T region_to_chunk_pos(const T& region_pos)
    { return region_pos * voxel_constants::chunk_region_size; }

    constexpr void for_v3i(const v3i& min_val, const v3i& max_val, const std::function<void(const v3i&)>& body)
    {
        for (v3i iter = min_val; iter.x < max_val.x; iter.x++)
        for (;                   iter.y < max_val.y; iter.y++)
        for (;                   iter.z < max_val.z; iter.z++)
            body(iter);
    }
}
