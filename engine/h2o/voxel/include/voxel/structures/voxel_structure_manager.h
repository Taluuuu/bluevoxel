#pragma once

#include "core/types.h"
#include "voxel_structure.h"

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace h2o
{
    class VoxelStructureManager
    {
    public:

        VoxelStructureManager() = default;

        [[nodiscard]] std::optional<u32> get_structure_id(const std::string& name) const;

        // Returns null if none could be found
        [[nodiscard]] const VoxelStructure* get_structure(u32 structure_id) const;
        [[nodiscard]] VoxelStructure* get_structure(u32 structure_id);

        // Ordered by ID
        [[nodiscard]] std::vector<std::string> structure_names() const;

        // Returns the new structure's ID
        u32 add_structure(const VoxelStructure& structure);
        // Returns the last valid structure's id
        std::optional<u32> delete_structure(u32 id);
        void rename_structure(u32 id, const std::string& name);

        // Serialization
        void save(const fs::path& path) const;
        bool load(const fs::path& path);

    private:

        std::vector< std::optional<VoxelStructure> > m_structures{};

    };

    struct VoxelStructureInstance
    {
        u32 id{};
        v3i position{};
    };
}
