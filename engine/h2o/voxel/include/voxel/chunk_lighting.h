#pragma once

#include "core/types.h"

#include <vector>

namespace h2o
{
    class ChunkLighting
    {
    public:

        ChunkLighting() = default;

        void init();

        [[nodiscard]] u8 get_light_level(const v3i& local_block_pos) const;
        void set_light_level(const v3i& local_block_pos, u8 light_level);

    private:

        std::vector<u8> m_light_levels{};

        bool m_is_initialized = false;

    };
}
