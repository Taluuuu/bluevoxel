#include "voxel/voxel_module.h"

#include "core/log.h"
#include "voxel/block.h"
#include "voxel/block_presets/block_preset_crop.h"
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
        register_block_preset("normal", std::make_shared<BlockPreset_Base>());
        register_block_preset("crop", std::make_shared<BlockPreset_Crop>());

        if (m_voxel_pack)
            set_block_types(load_block_types_from_voxel_pack(*m_voxel_pack));

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

    bool VoxelModule::is_valid_block_id(BlockID id) const
    {
        return id < block_type_count();
    }

    const BlockPreset_Base* VoxelModule::get_block_preset(BlockID id) const
    {
        return is_valid_block_id(id) ? m_block_presets_per_id[id] : nullptr;
    }

    std::optional<BlockPresetFlags> VoxelModule::get_block_preset_data(BlockID id) const
    {
        if (is_valid_block_id(id))
            return m_block_preset_flags_per_id[id];

        return std::nullopt;
    }

    void VoxelModule::register_block_preset(const std::string& name, const std::shared_ptr<BlockPreset_Base>& preset)
    {
        m_block_presets.emplace(name, preset);
    }

    void VoxelModule::set_block_types(const std::vector<std::optional<BlockType>>& block_types)
    {
        m_block_types = block_types;

        // Block presets init
        m_block_presets_per_id.clear();
        m_block_presets_per_id.resize(block_types.size());
        m_block_preset_flags_per_id.clear();
        m_block_preset_flags_per_id.resize(block_types.size());
        for (const auto& block_type : block_types)
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
    }

    std::vector<std::optional<BlockType>> VoxelModule::load_block_types_from_voxel_pack(const VoxelPack& voxel_pack)
    {
        std::vector<std::optional<BlockType>> result{};

        try
        {
            const auto root_yml = YAML::LoadFile(voxel_pack.block_types_path().string());
            const auto block_types_yml = root_yml["block_types"];
            for (const auto block_type_yml : block_types_yml)
            {
                const auto id = block_type_yml["id"].as<BlockID>();
                const auto name = block_type_yml["name"].as<std::string>();
                const auto preset_name = block_type_yml["preset"].as<std::string>();
                const auto model_name = block_type_yml["model"].as<std::string>();
                const auto texture_names = block_type_yml["textures"].as<std::vector<std::string>>();

                // This could be expanded upon if more block ids get reserved.
                if (id == 0)
                {
                    log::warn("Block type with name '{}' uses reserved id: {}.", name, id);
                    continue;
                }

                if (id >= result.size())
                    result.resize(id + 1);

                result[id] = { name, model_name, preset_name, texture_names, id };
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block types: {}", e.what());
        }

        return result;
    }
}