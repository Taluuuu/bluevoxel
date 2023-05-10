#pragma once

#include "core/module.h"
#include "core/types.h"

#include <unordered_map>
#include <vector>

namespace h2o
{
    class VoxelModule;

    class VoxelRenderingModule : public IModule
    {
    public:

        VoxelRenderingModule() = default;
        ~VoxelRenderingModule() override = default;

        bool init(Engine& engine) override;

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        std::unordered_map<std::string, std::vector<std::vector<u32>>> m_block_models;

        VoxelModule* m_voxel_module = nullptr;

    };
}