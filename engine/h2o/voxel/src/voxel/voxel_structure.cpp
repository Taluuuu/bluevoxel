#include "voxel/voxel_structure.h"

#include "core/log.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    VoxelStructure::VoxelStructure()
    {
        resize(v3i{ 1 });
    }

    Block VoxelStructure::get_block(const v3i& pos) const
    {
        assert(is_valid_pos(pos));
        return m_blocks[to_index(pos)];
    }

    bool VoxelStructure::is_valid_pos(const v3i& pos) const
    {
        return
            pos.x >= 0 && pos.x < m_size.x &&
            pos.y >= 0 && pos.y < m_size.y &&
            pos.z >= 0 && pos.z < m_size.z;
    }

    void VoxelStructure::resize(const v3i& new_size)
    {
        if (!is_valid_pos(new_size - v3i{ 1 }))
        {
            log::warn("Trying to resize voxel structure to invalid size: {}, {}, {}",
                new_size.x, new_size.y, new_size.z);

            return;
        }


    }

    void VoxelStructure::set_block(const v3i& pos, Block block)
    {
        assert(is_valid_pos(pos));
        m_blocks[to_index(pos)] = block;
    }

    size_t VoxelStructure::to_index(const v3i& pos) const
    {
        return
            pos.y * m_size.x * m_size.x +
            pos.x * voxel_constants::chunk_size +
            pos.z;
    }
}
