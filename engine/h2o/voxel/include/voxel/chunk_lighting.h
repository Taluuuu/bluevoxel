#pragma once

#include "chunk.h"
#include "core/types.h"
#include "grid/grid_view_3d.h"

#include <vector>

namespace h2o
{
    enum class ChunkLightingType
    {
        Light, Sunlight
    };

    struct ChunkLightLevel
    {
        u8 light    : 4 = 0;
        u8 sunlight : 4 = 0;

        [[nodiscard]] u8 get(const ChunkLightingType type) const
        {
            switch (type)
            {
            case ChunkLightingType::Light:    return light;
            case ChunkLightingType::Sunlight: return sunlight;
            }

            assert(false);
            return {};
        }

        void set(const ChunkLightingType type, const u8 light_level)
        {
            switch (type)
            {
            case ChunkLightingType::Light:    light = light_level;    break;
            case ChunkLightingType::Sunlight: sunlight = light_level; break;
            }
        }
    };

    class ChunkLighting
    {
    public:

        ChunkLighting();

        class ViewType : public View<ChunkLighting>
        {
        public:

            ViewType(const v3i& view_min, const v3i& view_size)
                : View(view_min, view_size) {}

            [[nodiscard]] ChunkLightLevel get_light_level(const v3i& block_pos, const EViewRelativeTo relative_to = EViewRelativeTo::World) const
            {
                if (const ChunkLighting* chunk_lighting = get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
                    return chunk_lighting->get_light_level(voxel_utils::block_pos_to_within_chunk(block_pos));

                return { voxel_constants::max_light_level, voxel_constants::max_light_level };
            }

            void set_light_level(const v3i& block_pos, const u8 light_level, const ChunkLightingType type, const EViewRelativeTo relative_to = EViewRelativeTo::World)
            {
                if (ChunkLighting* chunk_lighting = get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
                    chunk_lighting->set_light_level(voxel_utils::block_pos_to_within_chunk(block_pos), light_level, type);
            }
        };

        [[nodiscard]] ChunkLightLevel get_light_level(const v3i& local_block_pos) const
        {
            assert(Chunk::is_valid_pos(local_block_pos));
            return m_light_levels[Chunk::to_index(local_block_pos)];
        }

        void set_light_level(const v3i& local_block_pos, const u8 light_level, const ChunkLightingType type)
        {
            assert(Chunk::is_valid_pos(local_block_pos));
            m_light_levels[Chunk::to_index(local_block_pos)].set(type, light_level);
        }

        void reset();

    private:

        std::vector<ChunkLightLevel> m_light_levels{};

    };
}
