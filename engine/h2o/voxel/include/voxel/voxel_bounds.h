#pragma once

#include "core/types.h"

#include <functional>

namespace h2o
{
    class VoxelBounds
    {
    public:

        VoxelBounds(v2i center, i32 bounds_distance);

        void for_each_pos_in_bounds(const std::function<void(v2i)>& function) const;

        void set_bounds_distance(i32 bounds_distance);
        void set_bounds_center(v2i center);

        [[nodiscard]] bool in_bounds(v2i chunk_pos) const;
        [[nodiscard]] i32 bounds_distance() const { return m_bounds_distance; }

    private:

        v2i m_center{};
        i32 m_bounds_distance = 0;

    };
}