#pragma once

#include "core/types.h"

namespace h2o
{
    class VoxelBounds
    {
    public:

        VoxelBounds(v2i center, i32 bounds_distance);

        void set_bounds_distance(i32 bounds_distance);
        void set_bounds_center(v2i center);

        [[nodiscard]] bool in_bounds(v2i chunk_pos) const;

    private:

        v2i m_center{};
        i32 m_bounds_distance = 0;

    };
}