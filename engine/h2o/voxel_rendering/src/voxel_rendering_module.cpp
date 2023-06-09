#include "voxel_rendering/voxel_rendering_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "rendering/texture_array.h"
#include "voxel_rendering/block_model.h"
#include "voxel/direction.h"
#include "voxel/voxel_module.h"

#include <magic_enum.hpp>
#include <yaml-cpp/yaml.h>

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
                if (block_model_exists(name))
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

                add_block_model(name, std::move(model));
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block models: {}", e.what());
            return false;
        }

        u32 current_tex_index = 0;
        std::unordered_map<std::string, u32> texture_index_map;

        size_t block_type_count = m_voxel_module->block_type_count();
        m_block_model_indices_by_block_id.resize(block_type_count);
        m_texture_indices_by_block_id.resize(block_type_count);

        for (size_t i = 0; i < block_type_count; i++)
        {
            const BlockType* block_type = m_voxel_module->get_block_type(i);
            if (!block_type)
                continue;

            // Set model index
            auto model_index = get_model_index(block_type->model_name);
            if (!model_index.has_value())
            {
                log::error("Invalid block model '{}' requested for block '{}'.",
                    block_type->model_name, block_type->name);

                return false;
            }

            assert(i < m_block_model_indices_by_block_id.size());
            m_block_model_indices_by_block_id[i] = *model_index;

            // Set block textures
            const auto& tex_names = block_type->texture_names;
            std::vector<u32> block_tex_indices(tex_names.size(), 0);
            for (size_t j = 0; j < tex_names.size(); j++)
            {
                const auto& tex_name = tex_names[j];

                const auto tex_it = texture_index_map.find(tex_name);
                if (tex_it == texture_index_map.end())
                    texture_index_map[tex_name] = current_tex_index++;

                block_tex_indices[j] = texture_index_map[tex_name];
            }

            // TODO: Make sure the block type has the right amount of textures
            m_texture_indices_by_block_id[i] = std::move(block_tex_indices);
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
        auto it = m_block_model_indices_by_name.find(name);
        if (it == m_block_model_indices_by_name.end())
            return nullptr;

        assert(it->second < m_block_models.size());
        return &m_block_models[it->second];
    }

    const BlockModel* VoxelRenderingModule::get_model_safe(BlockID id) const
    {
        assert(id);
        if (id > m_block_model_indices_by_block_id.size())
            return nullptr;

        u32 model_index = m_block_model_indices_by_block_id[id];
        if (model_index > m_block_models.size())
            return nullptr;

        return &m_block_models[model_index];
    }

    const BlockModel* VoxelRenderingModule::get_model_fast(BlockID id) const
    {
        assert(id);
        assert(id < m_block_model_indices_by_block_id.size());
        u32 model_index = m_block_model_indices_by_block_id[id];
        assert(model_index < m_block_models.size());
        return &m_block_models[model_index];
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

    std::optional<u32> VoxelRenderingModule::get_model_index(const std::string& name) const
    {
        const auto it = m_block_model_indices_by_name.find(name);
        if (it == m_block_model_indices_by_name.end())
            return std::nullopt;

        return it->second;
    }

    bool VoxelRenderingModule::block_model_exists(const std::string& name) const
    {
        return get_model_index(name).has_value();
    }

    void VoxelRenderingModule::add_block_model(const std::string& name, BlockModel&& block_model)
    {
        // Does this block model already exist ?
        assert(!block_model_exists(name));

        const u32 new_index = m_block_models.size();
        m_block_models.emplace_back(std::move(block_model));

        m_block_model_indices_by_name[name] = new_index;
    }
}