#pragma once

#include "core/module.h"

namespace h2o
{
    class VoxelRenderingModule : public IModule
    {
    public:

        // IModule interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}