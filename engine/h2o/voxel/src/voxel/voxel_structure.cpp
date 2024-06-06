#include "voxel/voxel_structure.h"

#include "core/log.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    VoxelStructure::VoxelStructure(v3i size)
    {
        resize(size);
    }

    void VoxelStructure::set_block(const v3i& pos, Block block)
    {
        m_data.set_block(pos, block);
    }

    Block VoxelStructure::get_block(const v3i& pos) const
    {
        return m_data.get_block(pos);
    }

    void VoxelStructure::resize(const v3i& new_size)
    {
        if (new_size.x <= 0 ||
            new_size.y <= 0 ||
            new_size.z <= 0)
        {
            log::warn("Trying to resize voxel structure to invalid size: {}, {}, {}",
                new_size.x, new_size.y, new_size.z);

            return;
        }

        StructureData new_data(new_size);
        for (i32 i = 0; i < m_data.size.x; i++)
        {
            for (i32 j = 0; j < m_data.size.y; j++)
            {
                for (i32 k = 0; k < m_data.size.z; k++)
                {
                    const v3i pos{ i, j, k };
                    new_data.set_block(pos, m_data.get_block(pos));
                }
            }
        }

        m_data = new_data;
    }

    VoxelStructure::StructureData::StructureData(v3i size)
        : size(size)
    {
        blocks.resize(size.x * size.y * size.z, Block::Air);
    }

    void VoxelStructure::StructureData::set_block(const v3i& pos, Block block)
    {
        if (const auto index = to_index(pos))
            blocks[*index] = block;
    }

    Block VoxelStructure::StructureData::get_block(const v3i& pos) const
    {
        if (const auto index = to_index(pos))
            return blocks[*index];

        return Block::Air;
    }

    bool VoxelStructure::StructureData::is_valid_pos(const v3i& pos) const
    {
        return
            pos.x >= 0 && pos.x < size.x &&
            pos.y >= 0 && pos.y < size.y &&
            pos.z >= 0 && pos.z < size.z;
    }

    std::optional<size_t> VoxelStructure::StructureData::to_index(const v3i& pos) const
    {
        if (!is_valid_pos(pos))
            return std::nullopt;

        return
            pos.z * size.x * size.y +
            pos.y * size.x +
            pos.x;
    }
}
