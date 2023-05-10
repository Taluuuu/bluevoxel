#pragma once

#include "core/module.h"
#include "core/types.h"

#include <unordered_map>
#include <vector>

namespace h2o
{
    class VoxelModule;

    using BlockModel = std::vector<std::vector<u32>>;

    class VoxelRenderingModule : public IModule
    {
    public:

        VoxelRenderingModule() = default;
        ~VoxelRenderingModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] const BlockModel* get_model(const std::string& name) const;

    private:

        std::unordered_map< std::string, BlockModel > m_block_models;

        VoxelModule* m_voxel_module = nullptr;

    };
}