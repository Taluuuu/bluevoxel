#pragma once

#include "core/types.h"
#include "voxel_structure.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace h2o
{
    class VoxelStructureManager
    {
    public:

        VoxelStructureManager();

        [[nodiscard]] std::optional<u32> get_structure_id(const std::string& name) const;
        // Returns null if none could be found
        [[nodiscard]] const VoxelStructure* get_structure(u32 structure_id) const;
        [[nodiscard]] VoxelStructure* get_structure(u32 structure_id);

        [[nodiscard]] std::vector<std::string> structure_names() const;

        void add_structure(const std::string& name, const VoxelStructure& structure);

        void save(const fs::path& path) const;
        bool load(const fs::path& path);

    private:

        std::map<std::string, u32> m_structure_ids{};
        std::vector< std::optional<VoxelStructure> > m_structures{};

    };

    struct VoxelStructureInstance
    {
        u32 id{};
        v3i position{};
    };
}
