#include "voxel/voxel_module.h"

#include "core/log.h"
#include "core/types.h"
#include "voxel/block.h"

#include <optional>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    bool VoxelModule::init(Engine& engine)
    {
        std::vector<std::optional<BlockType>> result;

        try
        {
            const auto root = YAML::LoadFile("Resources/engine/voxels/block_types.yaml");
            const auto block_types = root["block_types"];
            for (const auto block_type : block_types)
            {
                const u16 id = block_type["id"].as<u16>();
                const auto name = block_type["name"].as<std::string>();

                // This could be expanded upon if more block ids get reserved.
                if (id == 0)
                {
                    log::warn("Block type with name '{}' uses reserved id: {}.", name, id);
                    continue;
                }

                const auto model_name = block_type["model"].as<std::string>();
                const auto texture_names = block_type["textures"].as<std::vector<std::string>>();

                if (id >= result.size())
                    result.resize(id + 1);

                result[id] = { name, model_name, texture_names };
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block types: {}", e.what());
            return false;
        }

        m_block_types = std::move(result);

        return true;
    }

    std::vector<std::type_index> VoxelModule::dependencies() const
    {
        return {};
    }

    const BlockType* VoxelModule::get_block_type(BlockID id) const
    {
        // Can't use an assert here as the data is loaded at runtime
        if (id >= m_block_types.size())
            return nullptr;

        if (const auto& block_type = m_block_types[id]; block_type.has_value())
            return &*block_type;

        return nullptr;
    }

    size_t VoxelModule::block_type_count() const
    {
        return m_block_types.size();
    }
}