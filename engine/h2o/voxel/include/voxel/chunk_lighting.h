#pragma once

#include "block.h"
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

            [[nodiscard]] ChunkLightLevel get_light_level(const v3i& block_pos, EViewRelativeTo relative_to = EViewRelativeTo::World) const;
            void set_light_level(const v3i& block_pos, u8 light_level, ChunkLightingType type, EViewRelativeTo relative_to = EViewRelativeTo::World);

        };

        [[nodiscard]] ChunkLightLevel get_light_level(const v3i& local_block_pos) const;
        void set_light_level(const v3i& local_block_pos, u8 light_level, ChunkLightingType type);

        void reset();

    private:

        std::vector<ChunkLightLevel> m_light_levels{};

    };
}
