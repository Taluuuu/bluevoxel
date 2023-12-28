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
    class VoxelPack;
    class RenderingModule;

    namespace gfx
    {
        class IPipeline;
        class TextureArray;
    }

    class VoxelRenderingModule : public IModule
    {
    public:

        explicit VoxelRenderingModule(const std::shared_ptr<VoxelPack>& voxel_pack);
        ~VoxelRenderingModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] const BlockModel* get_model(const std::string& name) const;
        [[nodiscard]] const BlockModel* get_model_safe(BlockID id) const;
        [[nodiscard]] const BlockModel* get_model_fast(BlockID id) const;
        [[nodiscard]] const std::vector<u32>& get_textures_fast(BlockID id) const;

        // Return value is always valid or an assert fails
        [[nodiscard]] const std::shared_ptr<gfx::IPipeline>& pipeline() const;
        [[nodiscard]] const std::shared_ptr<gfx::TextureArray>& block_textures() const;

    private:

        [[nodiscard]] std::optional<u32> get_model_index(const std::string& name) const;
        [[nodiscard]] bool block_model_exists(const std::string& name) const;
        void add_block_model(const std::string& name, BlockModel&& block_model);

    private:

        // Block models
        std::vector<BlockModel> m_block_models;
        std::unordered_map<std::string, u32> m_block_model_indices_by_name;
        std::vector<u32> m_block_model_indices_by_block_id;

        // Block textures
        std::vector<std::vector<u32>> m_texture_indices_by_block_id;

        // Rendering
        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;
        std::shared_ptr<gfx::TextureArray> m_block_textures = nullptr;

        // Module refs
        VoxelModule* m_voxel_module = nullptr;
        RenderingModule* m_rendering_module = nullptr;

        std::shared_ptr<VoxelPack> m_voxel_pack = nullptr;

    };
}