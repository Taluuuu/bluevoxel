#include "voxel/chunk_lighting.h"

#include "core/engine.h"
#include "voxel/chunk.h"

namespace h2o
{
    ChunkLighting::ChunkLighting()
    {
        m_light_levels.resize(voxel_constants::chunk_volume, {});
    }

    ChunkLightLevel ChunkLighting::ViewType::get_light_level(const v3i& block_pos, const EViewRelativeTo relative_to) const
    {
        if (const ChunkLighting* chunk_lighting = get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk_lighting->get_light_level(voxel_utils::block_pos_to_within_chunk(block_pos));

        return {};
    }

    void ChunkLighting::ViewType::set_light_level(
        const v3i& block_pos,
        const u8 light_level,
        const ChunkLightingType type,
        const EViewRelativeTo relative_to)
    {
        if (ChunkLighting* chunk_lighting = get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            chunk_lighting->set_light_level(voxel_utils::block_pos_to_within_chunk(block_pos), light_level, type);
    }

    ChunkLightLevel ChunkLighting::get_light_level(const v3i& local_block_pos) const
    {
        assert(Chunk::is_valid_pos(local_block_pos));
        return m_light_levels[Chunk::to_index(local_block_pos)];
    }

    void ChunkLighting::set_light_level(
        const v3i& local_block_pos,
        const u8 light_level,
        const ChunkLightingType type)
    {
        assert(Chunk::is_valid_pos(local_block_pos));
        m_light_levels[Chunk::to_index(local_block_pos)].set(type, light_level);
    }

    void ChunkLighting::reset()
    {
        std::ranges::fill(m_light_levels, ChunkLightLevel{});
    }
}
