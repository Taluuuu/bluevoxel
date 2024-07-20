#pragma once

#include "core/types.h"

#include <string>
#include <unordered_map>

#include "voxel_structure.h"

namespace h2o
{
    class VoxelStructureManager
    {
    public:

        VoxelStructureManager();

        [[nodiscard]] std::optional<u32> get_structure_id(const std::string& name) const;
        // Returns null if none could be found
        [[nodiscard]] const VoxelStructure* get_structure(u32 structure_id) const;

        void add_structure(const std::string& name, const VoxelStructure& structure);

    private:

        std::unordered_map<std::string, u32> m_structure_ids{};
        std::vector<VoxelStructure> m_structures{};

    };

    struct VoxelStructureInstance
    {
        u32 id{};
        v3i position{};
    };
}
