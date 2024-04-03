#pragma once

#include "core/events.h"
#include "core/module.h"
#include "core/types.h"
#include "voxel/block.h"
#include "voxel/block_model.h"

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

        VoxelRenderingModule() = default;
        ~VoxelRenderingModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] const BlockModel* get_model(BlockID id, const std::vector<u32>*& out_texture_ids) const;
        [[nodiscard]] bool is_transparent(BlockID id) const;

        // Return value is always valid or an assert fails
        [[nodiscard]] const std::shared_ptr<gfx::IPipeline>& pipeline() const;
        [[nodiscard]] const std::shared_ptr<gfx::TextureArray>& block_textures() const;

    private:

        // Rendering
        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;
        std::shared_ptr<gfx::TextureArray> m_block_textures = nullptr;

        // Module refs
        VoxelModule* m_voxel_module = nullptr;
        RenderingModule* m_rendering_module = nullptr;

        EventHandle m_on_voxel_pack_changed_handle{};

    };
}