#include "voxel_rendering/voxel_rendering_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "voxel/voxel_module.h"
#include "yaml-cpp/yaml.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "rendering/texture_array.h"
#include "voxel_rendering/block_model.h"
#include "voxel/direction.h"

#include <magic_enum.hpp>

namespace h2o
{
    std::vector<std::type_index> VoxelRenderingModule::dependencies() const
    {
        return { typeid(VoxelModule), typeid(RenderingModule) };
    }

    bool VoxelRenderingModule::init(Engine& engine)
    {
        m_voxel_module = engine.get_module<VoxelModule>();
        m_rendering_module = engine.get_module<RenderingModule>();
        if (!m_voxel_module || !m_rendering_module)
            return false;

        // Load block models
        try
        {
            const auto root = YAML::LoadFile("Resources/engine/voxels/block_models.yaml");
            const auto block_models = root["block_models"];
            for (const auto block_model : block_models)
            {
                const auto name = block_model["name"].as<std::string>();
                if (m_block_models.find(name) != m_block_models.end())
                {
                    log::warn("Multiple block models found with name: '{}'", name);
                    continue;
                }

                BlockModel model{};
                for (const auto face : block_model["faces"])
                {
                    const auto dir_name = face["dir"].as<std::string>();
                    const bool occluded = face["occluded"].as<bool>();
                    const auto vertices = face["vertices"].as<std::vector<std::array<u32, 5>>>();

                    const auto dir = magic_enum::enum_cast<voxel::Direction>(dir_name);
                    if (!dir || !magic_enum::enum_index(*dir))
                    {
                        log::error("Failed to import block model '{}'; invalid face direction: '{}'", name, dir_name);
                        continue;
                    }

                    const size_t face_idx = *magic_enum::enum_index(*dir);

                    auto& dir_vertices = occluded ?
                        model.occluded_vertices[face_idx] :
                        model.unoccluded_vertices[face_idx];

                    for (const auto& vertex : vertices)
                    {
                        dir_vertices.push_back(BlockVertex
                        {
                            .x = vertex[0],
                            .y = vertex[1],
                            .z = vertex[2],
                            .u = vertex[3],
                            .v = vertex[4],
                            .face_idx = static_cast<u32>(face_idx),
                        });
                    }
                }

                m_block_models[name] = std::move(model);
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block models: {}", e.what());
            return false;
        }

        auto& renderer = m_rendering_module->renderer();

        // Load textures
        m_block_textures = renderer.create_texture_array(1);
        if (!m_block_textures)
            return false;

        if (auto tex = renderer.fetch_or_load_texture("Resources/engine/textures/test.png"))
            m_block_textures->set_texture(0, tex);

        // Create rendering pipeline
        m_pipeline = renderer
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/chunk.vert")
            .add_shader(gfx::ShaderStage::Fragment, "Resources/engine/shaders/opengl/chunk.frag")
            .compile();

        if (!m_pipeline)
            return false;

        return true;
    }

    const BlockModel* VoxelRenderingModule::get_model(const std::string& name) const
    {
        auto it = m_block_models.find(name);
        return (it == m_block_models.end()) ? nullptr : &it->second;
    }

    const std::shared_ptr<gfx::IPipeline>& VoxelRenderingModule::pipeline() const
    {
        // The pipeline should be valid here, otherwise the init would have failed.
        assert(m_pipeline);
        return m_pipeline;
    }

    const std::shared_ptr<gfx::ITextureArray>& VoxelRenderingModule::block_textures() const
    {
        assert(m_block_textures);
        return m_block_textures;
    }
}