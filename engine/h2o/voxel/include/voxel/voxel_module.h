#pragma once

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

        [[nodiscard]] const BlockType* get_block_type(BlockID id) const;
        [[nodiscard]] size_t block_type_count() const;
        [[nodiscard]] bool is_valid_block_id(BlockID id) const;

        [[nodiscard]] const BlockPreset_Base* get_block_preset(BlockID id) const;
        [[nodiscard]] std::optional<BlockPresetFlags> get_block_preset_data(BlockID id) const;
        void register_block_preset(
            const std::string& name,
            const std::shared_ptr<BlockPreset_Base>& preset);

    protected:

        void set_block_types(const std::vector<std::optional<BlockType>>& block_types);

        static std::vector<std::optional<BlockType>> load_block_types_from_voxel_pack(const VoxelPack& voxel_pack);

    private:

        std::vector< std::optional<BlockType> > m_block_types;

        // Block presets
        std::unordered_map< std::string, std::shared_ptr<BlockPreset_Base> > m_block_presets;
        std::vector<BlockPreset_Base*> m_block_presets_per_id;
        std::vector<BlockPresetFlags> m_block_preset_flags_per_id;

        // TODO: Could the voxel pack load and store its own data?
        std::shared_ptr<VoxelPack> m_voxel_pack = nullptr;

    };
}