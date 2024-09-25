#include "voxel/structures/voxel_structure_manager.h"

#include "core/log.h"
#include "core/yaml.h"

#include <fstream>

namespace h2o
{
    std::optional<u32> VoxelStructureManager::get_structure_id(const std::string& name) const
    {
        for (u32 i = 0; i < m_structures.size(); i++)
        {
            const auto& structure = m_structures[i];
            if (structure && structure->name() == name)
                return i;
        }

        return std::nullopt;
    }

    const VoxelStructure* VoxelStructureManager::get_structure(u32 structure_id) const
    {
        if (structure_id < m_structures.size())
        {
            if (const auto& structure = m_structures[structure_id])
                return &*structure;
        }

        return nullptr;
    }

    VoxelStructure* VoxelStructureManager::get_structure(u32 structure_id)
    {
        if (structure_id < m_structures.size())
        {
            if (auto& structure = m_structures[structure_id])
                return &*structure;
        }

        return nullptr;
    }

    std::vector<std::string> VoxelStructureManager::structure_names() const
    {
        std::vector<std::string> result{};
        result.resize(m_structures.size(), "none");

        for (u32 i = 0; i < m_structures.size(); i++)
        {
            if (const auto& structure = m_structures[i])
                result[i] = structure->name();
        }

        return result;
    }

    u32 VoxelStructureManager::add_structure(const VoxelStructure& structure)
    {
        for (u32 i = 0; i < m_structures.size(); i++)
        {
            if (auto& s = m_structures[i]; !s)
            {
                s = structure;
                return i;
            }
        }

        const u32 id = m_structures.size();
        m_structures.emplace_back(structure);

        return id;
    }

    std::optional<u32> VoxelStructureManager::delete_structure(u32 id)
    {
        if (id < m_structures.size())
            m_structures[id] = std::nullopt;

        for (i32 i = i32(m_structures.size()) - 1; i >= 0; --i)
        {
            if (m_structures[i])
                return i;

            m_structures.erase(m_structures.begin() + i);
        }

        return std::nullopt;
    }

    void VoxelStructureManager::rename_structure(u32 id, const std::string& name)
    {
        if (id < m_structures.size())
        {
            if (auto& structure = m_structures[id])
                structure->set_name(name);
        }
    }

    void VoxelStructureManager::save(const fs::path& path) const
    {
        try
        {
            YAML::Emitter yaml{};

            yaml << YAML::BeginMap;

            yaml << YAML::Key << "structures";
            yaml << YAML::Value;

            yaml << YAML::BeginSeq;
            for (u32 id = 0; id < m_structures.size(); id++)
            {
                const VoxelStructure* structure = get_structure(id);
                if (!structure)
                    continue;

                const v3i structure_size = structure->size();

                yaml << YAML::BeginMap;

                yaml << YAML::Key << "name" << YAML::Value << structure->name();
                yaml << YAML::Key << "id" << YAML::Value << id;
                yaml << YAML::Key << "size" << YAML::Value << structure_size;

                // Output blocks as horizontal slices separated by blank lines so they are
                // somewhat readable
                yaml << YAML::Key << "blocks" << YAML::Value << YAML::Flow << YAML::BeginSeq;
                for (i32 j = 0; j < structure_size.y; j++)
                {
                    for (i32 i = 0; i < structure_size.x; i++)
                    {
                        yaml << YAML::Newline;
                        for (i32 k = 0; k < structure_size.z; k++)
                            yaml << structure->get_block({ i, j, k }).id;
                    }

                    yaml << YAML::Newline;
                }
                yaml << YAML::EndSeq;

                yaml << YAML::EndMap;
            }
            yaml << YAML::EndSeq;

            yaml << YAML::EndMap;

            std::ofstream file(path.string());
            file << yaml.c_str();

            log::info("Saved structures to file at '{}'", absolute(path).string());
        }
        catch (const std::exception& e)
        {
            log::error("Failed to save voxel structures: {}", e.what());
        }
    }

    bool VoxelStructureManager::load(const fs::path& path)
    {
        try
        {
            const auto root_yml = YAML::LoadFile(path.string());
            const auto structures_yml = root_yml["structures"];

            for (const auto structure_yml : structures_yml)
            {
                const auto name = structure_yml["name"].as<std::string>();
                const auto id = structure_yml["id"].as<BlockID>();
                const auto size = structure_yml["size"].as<v3i>();
                const auto blocks = structure_yml["blocks"].as<std::vector<u32>>();

                VoxelStructure structure{ name, size };
                for (u32 i = 0; i < blocks.size(); i++)
                {
                    const BlockID block = blocks[i];

                    const v3i pos{
                        (i / size.z) % size.x,
                        i / (size.x * size.z),
                        i % size.z
                    };

                    structure.set_block(pos, Block{ block });
                }

                if (id >= m_structures.size())
                    m_structures.resize(id + 1, std::nullopt);

                m_structures[id] = std::move(structure);
            }

            return true;
        }
        catch (const std::exception& e)
        {
            log::error("Failed to load voxel structures: {}", e.what());
            return false;
        }
    }
}
