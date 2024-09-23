#include "voxel/structures/voxel_structure_manager.h"

#include "core/log.h"
#include "core/yaml.h"

#include <fstream>

namespace h2o
{
    VoxelStructureManager::VoxelStructureManager()
    {
        // VoxelStructure tree({ 3, 5, 3 });
        //
        // tree.set_block({ 1, 0, 1 }, 6);
        // tree.set_block({ 1, 1, 1 }, 6);
        // tree.set_block({ 1, 2, 1 }, 6);
        // tree.set_block({ 1, 3, 1 }, 6);
        //
        // tree.set_block({ 0, 2, 0 }, 7);
        // tree.set_block({ 1, 2, 0 }, 7);
        // tree.set_block({ 2, 2, 0 }, 7);
        // tree.set_block({ 2, 2, 1 }, 7);
        // tree.set_block({ 2, 2, 2 }, 7);
        // tree.set_block({ 1, 2, 2 }, 7);
        // tree.set_block({ 0, 2, 2 }, 7);
        // tree.set_block({ 0, 2, 1 }, 7);
        //
        // tree.set_block({ 1, 3, 0 }, 7);
        // tree.set_block({ 2, 3, 1 }, 7);
        // tree.set_block({ 1, 3, 2 }, 7);
        // tree.set_block({ 0, 3, 1 }, 7);
        //
        // tree.set_block({ 1, 4, 1 }, 7);
        //
        // add_structure("tree", tree);
        //
        // VoxelStructure plus({ 32, 32, 32 });
        // for (i32 i = 0; i < 32; i++)
        // {
        //     plus.set_block({ i, 0, 0 }, 4);
        //     plus.set_block({ 0, i, 0 }, 4);
        //     plus.set_block({ 0, 0, i }, 4);
        // }
        //
        // add_structure("plus", plus);
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
        result.resize(m_structure_ids.size(), "none");

        for (const auto& [name, id] : m_structure_ids)
        {
            if (id < result.size())
                result[id] = name;
        }

        return result;
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

    void VoxelStructureManager::save(const fs::path& path) const
    {
        try
        {
            YAML::Emitter yaml{};

            yaml << YAML::BeginMap;

            yaml << YAML::Key << "structures";
            yaml << YAML::Value;

            yaml << YAML::BeginSeq;
            for (const auto& [name, id] : m_structure_ids)
            {
                const VoxelStructure* structure = get_structure(id);
                if (!structure)
                    continue;

                const v3i structure_size = structure->size();

                yaml << YAML::BeginMap;

                yaml << YAML::Key << "name" << YAML::Value << name;
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

                VoxelStructure structure{ size };
                for (u32 i = 0; i < blocks.size(); i++)
                {
                    const BlockID block = blocks[i];

                    const v3i pos{
                        (i / size.z) % size.x,//(i / size.x) % size.z,
                        i / (size.x * size.z),
                        i % size.z
                    };

                    structure.set_block(pos, Block{ block });
                }

                m_structure_ids[name] = id;

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
