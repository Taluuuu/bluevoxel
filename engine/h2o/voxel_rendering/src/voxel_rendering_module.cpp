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
#include "voxel/voxel_pack.h"

#include <magic_enum.hpp>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    VoxelRenderingModule::VoxelRenderingModule(const std::shared_ptr<VoxelPack>& voxel_pack)
        : m_voxel_pack(voxel_pack)
    {}

    std::vector<std::type_index> VoxelRenderingModule::dependencies() const
    {
        return { typeid(VoxelModule), typeid(RenderingModule) };
    }

    bool VoxelRenderingModule::init(Engine& engine)
    {
        if (!m_voxel_pack)
        {
            log::error("No usable voxel pack found in voxel rendering module.");
            return false;
        }

        m_voxel_module = engine.get_module<VoxelModule>();
        m_rendering_module = engine.get_module<RenderingModule>();
        if (!m_voxel_module || !m_rendering_module)
            return false;

        // Load block models
        try
        {
            const auto root = YAML::LoadFile(m_voxel_pack->block_models_path().string());
            const auto block_models = root["block_models"];
            for (const auto block_model : block_models)
            {
                const auto name = block_model["name"].as<std::string>();
                if (block_model_exists(name))
                {
                    log::warn("Multiple block models found with name: '{}'", name);
                    continue;
                }

                // An index that keeps track of the order in which faces are defined.
                // This is a bit of a hack as the tex_idx defined here is not the same as the tex_idx that
                // is sent to the gpu, which is defined per-block.
                u32 tex_idx = 0;

                BlockModel model{};
                for (const auto face_yml : block_model["faces"])
                {
                    const auto vertices = face_yml["vertices"].as<std::vector<std::array<u32, 5>>>();
                    if (vertices.size() % 3 != 0)
                    {
                        log::error("Number of vertices for faces in block model '{}' must be a multiple of 3.", name);
                        return false;
                    }

                    const auto occluded_by_yml = face_yml["occluded_by"];

                    std::vector<BlockVertex> face_vertices;
                    face_vertices.reserve(vertices.size());
                    for (const auto& vertex : vertices)
                    {
                        face_vertices.push_back(BlockVertex
                            {
                                .x = vertex[0],
                                .y = vertex[1],
                                .z = vertex[2],
                                .u = vertex[3],
                                .v = vertex[4],
                                .tex_idx = tex_idx
                            });
                    }

                    tex_idx++;

                    if (occluded_by_yml)
                    {
                        // An occluder is specified
                        const auto occluder_name = occluded_by_yml.as<std::string>();
                        const auto occluder = magic_enum::enum_cast<voxel::Direction>(occluder_name);

                        if (!occluder)
                        {
                            log::error("Failed to import block model '{}'; invalid occluder name: '{}'", name, occluder_name);
                            continue;
                        }

                        const auto dir_idx = magic_enum::enum_index(*occluder);
                        assert(dir_idx);

                        model.occluded_vertices[*dir_idx].emplace_back(std::move(face_vertices));
                    }
                    else
                    {
                        model.unoccluded_vertices.emplace_back(std::move(face_vertices));
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

        i32 current_tex_index = 0;
        std::unordered_map<std::string, i32> texture_index_map;

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

            const auto model = get_model_safe(i);
            assert(model);
            size_t num_model_faces = model->unoccluded_vertices.size();
            for (const auto& faces : model->occluded_vertices)
                num_model_faces += faces.size();

            if (num_model_faces != tex_names.size())
            {
                log::error("Mismatch between number of faces and number of textures for block: {}", block_type->name);
                return false;
            }

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
        m_block_textures = renderer.create_texture_array(texture_index_map.size());
        if (!m_block_textures)
            return false;

        for (const auto& [tex_name, tex_index] : texture_index_map)
        {
            auto tex = renderer.fetch_or_load_texture(
                (m_voxel_pack->textures_path() / fs::path(tex_name)).string());

            if (!tex)
                continue;

            assert(tex_index < texture_index_map.size());
            m_block_textures->set_texture(tex_index, tex);
        }

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

    const std::vector<u32>& VoxelRenderingModule::get_textures_fast(BlockID id) const
    {
        assert(id);
        assert(id < m_texture_indices_by_block_id.size());
        return m_texture_indices_by_block_id[id];
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