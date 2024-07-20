#include "voxel/structures/voxel_structure_manager.h"

#include "core/log.h"

namespace h2o
{
    VoxelStructureManager::VoxelStructureManager()
    {
        VoxelStructure tree({ 3, 5, 3 });

        tree.set_block({ 1, 0, 1 }, 6);
        tree.set_block({ 1, 1, 1 }, 6);
        tree.set_block({ 1, 2, 1 }, 6);
        tree.set_block({ 1, 3, 1 }, 6);

        tree.set_block({ 0, 2, 0 }, 7);
        tree.set_block({ 1, 2, 0 }, 7);
        tree.set_block({ 2, 2, 0 }, 7);
        tree.set_block({ 2, 2, 1 }, 7);
        tree.set_block({ 2, 2, 2 }, 7);
        tree.set_block({ 1, 2, 2 }, 7);
        tree.set_block({ 0, 2, 2 }, 7);
        tree.set_block({ 0, 2, 1 }, 7);

        tree.set_block({ 1, 3, 0 }, 7);
        tree.set_block({ 2, 3, 1 }, 7);
        tree.set_block({ 1, 3, 2 }, 7);
        tree.set_block({ 0, 3, 1 }, 7);

        tree.set_block({ 1, 4, 1 }, 7);

        add_structure("tree", tree);
    }

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

    void VoxelStructureManager::add_structure(const std::string& name, const VoxelStructure& structure)
    {
        if (get_structure_id(name).has_value())
        {
            log::warn("There is already a structure registered with name '{}'. Ignoring.", name);
            return;
        }

        m_structure_ids.emplace(name, m_structures.size());
        m_structures.push_back(structure);

        assert(m_structures.size() == m_structure_ids.size());
    }
}
