#pragma once

#include "core/module.h"
#include "core/types.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class VoxelModule;
    class RenderingModule;

    namespace gfx { class IPipeline; }

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

        [[nodiscard]] const gfx::IPipeline& pipeline() const;

    private:

        std::unordered_map< std::string, BlockModel > m_block_models;

        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;

        VoxelModule* m_voxel_module = nullptr;
        RenderingModule* m_rendering_module = nullptr;

    };
}