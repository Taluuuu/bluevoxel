#include "voxel_rendering/voxel_rendering_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "voxel/block_model.h"
#include "voxel/direction.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

#include <magic_enum.hpp>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    std::vector<std::type_index> VoxelRenderingModule::dependencies() const
    {
        return {
            typeid(VoxelModule),
            typeid(RenderingModule) };
    }

    bool VoxelRenderingModule::init(Engine& engine)
    {
        m_voxel_module = &engine.get_module_checked<VoxelModule>();
        m_rendering_module = &engine.get_module_checked<RenderingModule>();

        // Create rendering pipeline
        m_pipeline = m_rendering_module->renderer()
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex,   "../Resources/engine/shaders/chunk.vert")
            .add_shader(gfx::ShaderStage::Fragment, "../Resources/engine/shaders/chunk.frag")
            .with_feature(gfx::PipelineFeature::CullFace)
            .with_feature(gfx::PipelineFeature::DepthTest)
            .compile();

        if (!m_pipeline)
            return false;

        m_voxel_module->on_voxel_pack_changed.add_listener(m_on_voxel_pack_changed_handle,
            [this](const VoxelPackChangedEvent& event)
            {
                const auto& voxel_pack = event.voxel_pack;
                const auto& texture_name_id_map = voxel_pack.texture_ids();

                m_block_textures = m_rendering_module->renderer()
                    .create_texture_array_ptr(texture_name_id_map.size());

                for (const auto& [name, id] : texture_name_id_map)
                {
                    if (auto tex = g_engine->resource_mgr().fetch<gfx::Texture>(
                        (voxel_pack.path() / VoxelPack::textures_folder_name / fs::path(name)).string()))
                    {
                        m_block_textures->set_texture(id, tex);
                    }
                }
            }
        );

        return true;
    }

    const BlockModel* VoxelRenderingModule::get_model(BlockID id, const std::vector<u32>*& out_texture_ids) const
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return nullptr;

        const auto& block_types = voxel_pack->block_types();

        if (id >= block_types.size())
            return nullptr;

        if (const auto& block_type = block_types[id])
        {
            out_texture_ids = &block_type->texture_ids;
            return &voxel_pack->block_models()[block_type->model_id];
        }

        return nullptr;
    }

    const std::shared_ptr<gfx::IPipeline>& VoxelRenderingModule::pipeline() const
    {
        // The pipeline should be valid here, otherwise the init would have failed.
        assert(m_pipeline);
        return m_pipeline;
    }

    const std::shared_ptr<gfx::TextureArray>& VoxelRenderingModule::block_textures() const
    {
        return m_block_textures;
    }
}