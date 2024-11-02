#include "voxel/chunk_lighting.h"

#include "voxel/voxel_constants.h"

namespace h2o
{
    // TODO: These are the same as in Chunk.cpp, move them somewhere common?
    static constexpr bool is_valid_pos(const v3i& local_pos)
    {
        return
            local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
            local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
            local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
    }

    static constexpr size_t to_index(const v3i& local_pos)
    {
        return
            local_pos.y * voxel_constants::chunk_area +
            local_pos.x * voxel_constants::chunk_size +
            local_pos.z;
    }

    void ChunkLighting::init()
    {
        assert(!m_is_initialized);

        // Two light levels are stored per u8
        static_assert(voxel_constants::num_light_level_bits == 4, "ChunkLighting class expects light level bits to be 4");
        m_light_levels.resize(voxel_constants::chunk_volume, 0);

        m_is_initialized = true;
    }

    u8 ChunkLighting::get_light_level(const v3i& local_block_pos) const
    {
        assert(m_is_initialized);
        assert(is_valid_pos(local_block_pos));

        return m_light_levels[to_index(local_block_pos)];
    }

    void ChunkLighting::set_light_level(const v3i& local_block_pos, const u8 light_level)
    {
        assert(m_is_initialized);
        assert(is_valid_pos(local_block_pos));

        m_light_levels[to_index(local_block_pos)] = light_level;
    }
}
