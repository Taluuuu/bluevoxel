#include "voxel_rendering/voxel_rendering_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "voxel/voxel_module.h"
#include "yaml-cpp/yaml.h"
#include "rendering/rendering_module.h"

namespace h2o
{
    std::vector<std::type_index> VoxelRenderingModule::dependencies() const
    {
        return { typeid(VoxelModule), typeid(RenderingModule) };
    }

    bool VoxelRenderingModule::init(Engine& engine)
    {
        m_voxel_module = engine.get_module<VoxelModule>();
        if (!m_voxel_module)
            return false;

        // Load block models
        try
        {
            const auto root = YAML::LoadFile("Resources/engine/voxels/block_models.yaml");
            const auto block_models = root["block_models"];
            for (const auto block_model : block_models)
            {
                const auto name = block_model["name"].as<std::string>();
                const auto vertices = block_model["vertices"].as<std::vector<std::vector<u32>>>();

                if (m_block_models.find(name) != m_block_models.end())
                {
                    log::warn("Multiple block models found with name: '{}'", name);
                    continue;
                }

                m_block_models[name] = vertices;
            }
        }
        catch(const std::exception& e)
        {
            log::error("Failed to import block models: {}", e.what());
            return false;
        }

        return true;
    }

    const BlockModel* VoxelRenderingModule::get_model(const std::string& name) const
    {
        auto it = m_block_models.find(name);
        return (it == m_block_models.end()) ? nullptr : &it->second;
    }
}