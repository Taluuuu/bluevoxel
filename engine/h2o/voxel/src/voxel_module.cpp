#include "voxel/voxel_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "voxel/block_presets/block_preset_crop.h"
#include "voxel/voxel_pack.h"

#include <yaml-cpp/yaml.h>

namespace h2o
{
    bool VoxelModule::init(Engine& engine)
    {
        register_block_preset("normal", std::make_shared<BlockPreset_Base>());
        register_block_preset("crop", std::make_shared<BlockPreset_Crop>());

        return true;
    }

    std::vector<std::type_index> VoxelModule::dependencies() const
    {
        return {};
    }

    void VoxelModule::set_voxel_pack(const std::shared_ptr<VoxelPack>& voxel_pack)
    {
        m_voxel_pack = voxel_pack;

        if (voxel_pack)
            on_voxel_pack_changed.broadcast({ *voxel_pack });
    }

    std::optional<u32> VoxelModule::find_block_preset_id(const std::string& preset_name) const
    {
        u32 index = 0;
        for (const auto& preset : m_block_presets)
        {
            if (preset.name == preset_name)
                return index;

            index++;
        }

        return std::nullopt;
    }

    const std::string* VoxelModule::find_block_preset_name(u32 preset_id) const
    {
        if (preset_id >= m_block_presets.size())
            return nullptr;

        return &m_block_presets[preset_id].name;
    }

    void VoxelModule::register_block_preset(const std::string& name, const std::shared_ptr<BlockPreset_Base>& preset)
    {
        m_block_presets.emplace_back(name, preset);
    }
}