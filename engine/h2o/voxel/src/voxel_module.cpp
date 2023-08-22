#include "voxel/voxel_module.h"

#include "core/log.h"
#include "voxel/block.h"
#include "voxel/voxel_pack.h"

#include <optional>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    VoxelModule::VoxelModule(const std::shared_ptr<VoxelPack>& voxel_pack)
        : m_voxel_pack(voxel_pack)
    {}

    bool VoxelModule::init(Engine& engine)
    {
        if (!m_voxel_pack)
        {
            log::error("No usable voxel pack found in voxel module.");
            return false;
        }

        register_block_preset("normal", std::make_unique<BlockPreset_Base>());

        std::vector<std::optional<BlockType>> result;

        try
        {
            const auto root = YAML::LoadFile(m_voxel_pack->block_types_path().string());
            const auto block_types = root["block_types"];
            for (const auto block_type : block_types)
            {
                const auto id = block_type["id"].as<BlockID>();
                const auto name = block_type["name"].as<std::string>();
                const auto preset_name = block_type["preset"].as<std::string>();

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

                result[id] = { name, model_name, preset_name, texture_names, id };
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block types: {}", e.what());
            return false;
        }

        // Block presets ini
        m_block_presets_per_id.resize(result.size());
        m_block_preset_flags_per_id.resize(result.size());
        for (const auto& block_type : result)
        {
            if (!block_type.has_value())
                continue;

            const auto block_id = block_type->block_id;
            const auto preset_name = block_type->block_preset_name;

            const auto preset_it = m_block_presets.find(preset_name);
            if (preset_it == m_block_presets.end() || !preset_it->second)
            {
                log::warn("No block preset with name '{}' found.", preset_name);
                continue;
            }

            m_block_presets_per_id[block_id] = preset_it->second.get();
            m_block_preset_flags_per_id[block_id] = preset_it->second->preset_flags();
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

    const BlockPreset_Base* VoxelModule::get_block_preset(BlockID id) const
    {
        return m_block_presets_per_id[id];
    }

    BlockPresetFlags VoxelModule::get_block_preset_data(BlockID id) const
    {
        return m_block_preset_flags_per_id[id];
    }

    void VoxelModule::register_block_preset(const std::string& name, std::shared_ptr<BlockPreset_Base>&& preset)
    {
        m_block_presets.emplace(name, std::move(preset));
    }
}