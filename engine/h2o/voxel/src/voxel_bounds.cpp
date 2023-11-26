#include "voxel/voxel_bounds.h"

namespace h2o
{
    VoxelBounds::VoxelBounds(v2i center, i32 bounds_distance)
    {
        set_bounds_center(center);
        set_bounds_distance(bounds_distance);
    }

    void VoxelBounds::for_each_pos_in_bounds(const std::function<void(v2i)>& function) const
    {
        for (i32 i = -m_bounds_distance; i <= m_bounds_distance; i++)
        for (i32 j = -m_bounds_distance; j <= m_bounds_distance; j++)
        {
            function(m_center + v2i{ i, j });
        }
    }

    void VoxelBounds::set_bounds_distance(i32 bounds_distance)
    {
        m_bounds_distance = bounds_distance;
    }

    void VoxelBounds::set_bounds_center(v2i center)
    {
        m_center = center;
    }

    bool VoxelBounds::in_bounds(v2i chunk_pos) const
    {
        const v2i min = m_center - v2i{ m_bounds_distance, m_bounds_distance };
        const v2i max = m_center + v2i{ m_bounds_distance, m_bounds_distance };

        return
            chunk_pos.x >= min.x && chunk_pos.x <= max.x &&
            chunk_pos.y >= min.y && chunk_pos.y <= max.y;
    }
}