#pragma once

#include "block.h"
#include "core/types.h"

#include <functional>
#include <vector>

namespace h2o
{
    class ChunkLighting
    {
    public:

        ChunkLighting();

        [[nodiscard]] u8 get_light_level(const v3i& local_block_pos) const;
        void set_light_level(const v3i& local_block_pos, u8 light_level);

        void reset();

        // Can't use ChunkView directly here since chunk_manager.h includes this file
        void update_lighting(const std::function<Block(const v3i&)>& get_block_function);

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
