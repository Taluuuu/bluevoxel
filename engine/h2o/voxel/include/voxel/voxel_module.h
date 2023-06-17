#pragma once

#include "core/module.h"
#include "voxel/block.h"

#include <memory>
#include <optional>
#include <vector>

namespace h2o
{
    class VoxelPack;

    class VoxelModule : public IModule
    {
    public:

        explicit VoxelModule(const std::shared_ptr<VoxelPack>& voxel_pack);

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] const BlockType* get_block_type(BlockID id) const;
        [[nodiscard]] size_t block_type_count() const;

    private:

        std::vector<std::optional<BlockType>> m_block_types;

        std::shared_ptr<VoxelPack> m_voxel_pack = nullptr;

    };
}