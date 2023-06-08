#pragma once

#include "core/module.h"
#include "core/types.h"
#include "voxel/block.h"
#include "voxel_rendering/block_model.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class VoxelModule;
    class RenderingModule;

    namespace gfx
    {
        class IPipeline;
        class ITextureArray;
    }

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
        [[nodiscard]] const BlockModel* get_model(BlockID id) const;

        // Return value is always valid or an assert fails
        [[nodiscard]] const std::shared_ptr<gfx::IPipeline>& pipeline() const;
        [[nodiscard]] const std::shared_ptr<gfx::ITextureArray>& block_textures() const;

    private:

        std::unordered_map< std::string, BlockModel > m_block_models;

        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;
        std::shared_ptr<gfx::ITextureArray> m_block_textures = nullptr;

        VoxelModule* m_voxel_module = nullptr;
        RenderingModule* m_rendering_module = nullptr;

    };
}