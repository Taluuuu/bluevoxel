#pragma once

#include "block.h"
#include "core/types.h"
#include "grid/grid_view_3d.h"

#include <functional>
#include <vector>

namespace h2o
{
    class ChunkLighting
    {
    public:

        ChunkLighting();

        class ViewType : public View<ChunkLighting>
        {
        public:

            ViewType(const v3i& view_min, const v3i& view_size)
                : View(view_min, view_size) {}

            [[nodiscard]] u8 get_light_level(const v3i& block_pos, EViewRelativeTo relative_to = EViewRelativeTo::World) const;
            void set_light_level(const v3i& block_pos, u8 light_level, EViewRelativeTo relative_to = EViewRelativeTo::World);

        };

        [[nodiscard]] u8 get_light_level(const v3i& local_block_pos) const;
        void set_light_level(const v3i& local_block_pos, u8 light_level);

        void reset();

    private:

        struct LightLevel
        {
            u8 light    : 4 = 0;
            u8 sunlight : 4 = 0;

            [[nodiscard]] u8 calc() const { return glm::max(light, sunlight); }
        };

        std::vector<LightLevel> m_light_levels{};

    };
}
