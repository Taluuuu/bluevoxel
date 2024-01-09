#pragma once

#include "core/events.h"
#include "core/module.h"
#include "voxel/block.h"
#include "voxel/block_presets/block_preset_base.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class BlockPreset_Base;
    class VoxelPack;

    struct VoxelPackChangedEvent
    {
        const VoxelPack& voxel_pack;
    };

    class VoxelModule : public IModule
    {
    public:

        VoxelModule() = default;
        ~VoxelModule() override = default;

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] const std::shared_ptr<VoxelPack>& voxel_pack() const { return m_voxel_pack; }
        void set_voxel_pack(const std::shared_ptr<VoxelPack>& voxel_pack);

        struct BlockPresetData
        {
            std::string name{};
            std::shared_ptr<BlockPreset_Base> preset{};
        };

        [[nodiscard]] const std::vector<BlockPresetData>& block_presets() const { return m_block_presets; }
        [[nodiscard]] std::optional<u32> find_block_preset_id(const std::string& preset_name) const;
        [[nodiscard]] const std::string* find_block_preset_name(u32 preset_id) const;
        void register_block_preset(const std::string& name, const std::shared_ptr<BlockPreset_Base>& preset);

    public:

        Event<VoxelPackChangedEvent> on_voxel_pack_changed{};

    private:

        std::vector<BlockPresetData> m_block_presets;

        std::shared_ptr<VoxelPack> m_voxel_pack = nullptr;

    };
}