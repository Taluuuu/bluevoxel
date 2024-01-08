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

    struct OnVoxelPackChanged
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

//        [[nodiscard]] const BlockType* get_block_type(BlockID id) const;
//        [[nodiscard]] size_t block_type_count() const;
//        [[nodiscard]] bool is_valid_block_id(BlockID id) const;

//        [[nodiscard]] const BlockPreset_Base* get_block_preset(BlockID id) const;
//        [[nodiscard]] std::optional<BlockPresetFlags> get_block_preset_data(BlockID id) const;
        [[nodiscard]] std::optional<u32> find_block_preset_id(const std::string& preset_name) const;
        void register_block_preset(
            const std::string& name,
            const std::shared_ptr<BlockPreset_Base>& preset);

    public:

        Event<OnVoxelPackChanged> on_voxel_pack_changed{};

    private:

        // Block presets
        struct BlockPresetData
        {
            std::string name{};
            std::shared_ptr<BlockPreset_Base> preset{};
        };
        std::vector<BlockPresetData> m_block_presets;

        std::shared_ptr<VoxelPack> m_voxel_pack = nullptr;

    };
}