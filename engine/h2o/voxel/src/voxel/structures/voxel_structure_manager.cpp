#include "voxel/structures/voxel_structure_manager.h"

namespace h2o
{
    std::optional<u32> VoxelStructureManager::get_structure_id(const std::string& name) const
    {
        if (const auto it = m_structure_ids.find(name); it != m_structure_ids.end())
            return it->second;

        return std::nullopt;
    }

    const VoxelStructure* VoxelStructureManager::get_structure(u32 structure_id) const
    {
        if (structure_id < m_structures.size())
            return &m_structures[structure_id];

        return nullptr;
    }
}
