#include "voxel/voxel_module.h"

#include "core/engine.h"
#include "networking/networking_module.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/renderer_enums.h"
#include "rendering/rendering_module.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "voxel/traits/block_trait.h"
#include "voxel/traits/block_trait_rotation.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/inventory_manager_voxel.h"

namespace h2o
{
    VoxelModule::VoxelModule()
        : Tickable(g_engine)
    {
    }

    bool VoxelModule::init(Engine& engine)
    {
        m_inventory_manager = std::make_shared<InventoryManager_Voxel>(*this);

        // Register traits here
        m_block_trait_manager.register_trait<BlockTrait_Rotation>("rotation");

        // Rendering module is an optional dependency.
        m_rendering_module = engine.get_module<RenderingModule>();
        if (m_rendering_module)
        {
            // Create rendering pipeline
            m_pipeline = m_rendering_module->renderer()
                .create_pipeline()
                .add_shader(gfx::ShaderStage::Vertex,   "engine/shaders/chunk.vert")
                .add_shader(gfx::ShaderStage::Fragment, "engine/shaders/chunk.frag")
                .with_feature(gfx::PipelineFeature::CullFace)
                .with_feature(gfx::PipelineFeature::DepthTest)
                .compile();

            return m_pipeline != nullptr;
        }

        return true;
    }

    std::vector<std::type_index> VoxelModule::dependencies() const
    {
        return {};
    }

    std::vector<std::type_index> VoxelModule::optional_dependencies() const
    {
        return { typeid(NetworkingModule), typeid(RenderingModule) };
    }

    void VoxelModule::set_voxel_pack(const std::shared_ptr<VoxelPack>& voxel_pack)
    {
        m_voxel_pack = voxel_pack;

        if (voxel_pack)
        {
            if (with_rendering())
            {
                const auto& texture_name_id_map = voxel_pack->texture_ids();

                m_block_textures = m_rendering_module->renderer()
                    .create_texture_array_ptr(texture_name_id_map.size());

                for (const auto& [name, id] : texture_name_id_map)
                {
                    if (auto tex = g_engine->resource_mgr().fetch<gfx::Texture>(
                        (voxel_pack->path() / VoxelPack::textures_folder_name / fs::path(name)).string()))
                    {
                        m_block_textures->set_texture(id, tex);
                    }
                }
            }

            on_voxel_pack_changed.broadcast({ *voxel_pack });
        }
    }

    std::optional<BlockModel> VoxelModule::get_model(const Block block, const std::vector<u32>*& out_texture_ids) const
    {
        assert(with_rendering());

        if (!m_voxel_pack)
            return std::nullopt;

        if (const auto block_type = m_voxel_pack->get_block_type(block.id))
        {
            if (const auto block_model = m_voxel_pack->get_block_model(block_type->model_id))
            {
                // AAAAAAAAAARGHHHHHH
                // There are like 8 heap allocations here
                auto edited_block_model = *block_model;

                block_type->for_each_trait(
                    [&](const BlockTrait& trait)
                    {
                        trait.edit_block_model(block, edited_block_model);
                    }
                );

                out_texture_ids = &block_type->texture_ids;
                return edited_block_model;
            }
        }

        return std::nullopt;
    }

    bool VoxelModule::is_transparent(BlockID id) const
    {
        if (m_voxel_pack)
        {
            // TODO: This is bad, id is not checked
            if (const auto& block_type = m_voxel_pack->block_types()[id])
                return block_type->is_transparent;
        }

        return true;
    }

    const std::shared_ptr<gfx::IPipeline>& VoxelModule::pipeline() const
    {
        // The pipeline should be valid here, otherwise the init would have failed.
        assert(m_pipeline && with_rendering());
        return m_pipeline;
    }

    const std::shared_ptr<gfx::TextureArray>& VoxelModule::block_textures() const
    {
        assert(with_rendering());
        return m_block_textures;
    }
}
