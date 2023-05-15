#include "voxel_rendering/voxel_rendering_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "voxel/voxel_module.h"
#include "yaml-cpp/yaml.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
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

                BlockModel model;
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

                    const size_t dir_index = *magic_enum::enum_index(*dir);
                    if (occluded)
                    {
                        model.occluded_vertices[dir_index];
                    }
                    else
                    {

                    }
                }

//                m_block_models[name] = ;
            }
        }
        catch(const std::exception& e)
        {
            log::error("Failed to import block models: {}", e.what());
            return false;
        }

        // Create rendering pipeline
        auto& renderer = m_rendering_module->renderer();
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
}