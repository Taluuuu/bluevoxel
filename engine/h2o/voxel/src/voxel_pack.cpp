#include "voxel/voxel_pack.h"

#include "core/engine.h"
#include "voxel/voxel_module.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    void VoxelPack::edit_block_type(BlockID block_id, BlockType edited_block_type)
    {
        // Do some validations here
        assert(block_id < m_block_types.size());
        assert(edited_block_type.model_id < m_block_models.size());

        // TODO: Test this with 0 textures
        const u32 face_count = m_block_models[edited_block_type.model_id].calculate_face_count();
        edited_block_type.texture_ids.resize(face_count, 0);

        m_block_types[block_id] = edited_block_type;

        on_voxel_pack_updated.broadcast({ *this });

        m_is_dirty = true;
    }

    void VoxelPack::save() const
    {
        if (!is_dirty())
            return;

        try
        {
            YAML::Emitter yaml{};

            yaml << YAML::BeginMap;

                yaml << YAML::Key << "block_types";
                yaml << YAML::Value;

                yaml << YAML::BeginSeq;

                    for (const auto& block_type : m_block_types)
                    {
                        if (!block_type)
                            continue;

                        yaml << YAML::BeginMap;

                            yaml << YAML::Key << "name";
                            yaml << YAML::Value << block_type->name;

                            auto& voxel_module = g_engine->get_module_checked<VoxelModule>();
                            if (const auto preset_name = voxel_module.find_block_preset_name(block_type->preset_id))
                            {
                                yaml << YAML::Key << "preset";
                                yaml << YAML::Value << *preset_name;
                            }

                            yaml << YAML::Key << "id";
                            yaml << YAML::Value << block_type->block_id;

                            yaml << YAML::Key << "model";
                            yaml << YAML::Value << m_block_models[block_type->model_id].name;

                            yaml << YAML::Key << "textures";
                            yaml << YAML::Value;

                            yaml << YAML::BeginSeq;

                                for (u32 texture_id : block_type->texture_ids)
                                {
                                    // Find texture name with id texture_id
                                    const auto it = std::find_if(m_texture_ids.begin(), m_texture_ids.end(),
                                        [&](const auto& item)
                                        { return item.second == texture_id; }
                                    );

                                    if (it != m_texture_ids.end())
                                        yaml << it->first;
                                }

                            yaml << YAML::EndSeq;

                        yaml << YAML::EndMap;
                    }

                yaml << YAML::EndSeq;

            yaml << YAML::EndMap;

            const auto block_types_path = m_path / block_types_file_name;
            std::ofstream file(block_types_path.string());
            file << yaml.c_str();

            log::info("Saved block types to file at '{}'", absolute(block_types_path).string());
        }
        catch(const std::exception& e)
        {
            log::error("Failed to save voxel pack: {}", e.what());
        }
    }

    bool VoxelPack::load(const fs::path& path)
    {
        const fs::path block_types_path = path / block_types_file_name;
        if (!exists(block_types_path))
        {
            log::error("Block types path at '{}' does not exist.", block_types_path.string());
            return false;
        }

        const fs::path block_models_path = path / block_models_file_name;
        if (!exists(block_models_path))
        {
            log::error("Block models path at '{}' does not exist.", block_models_path.string());
            return false;
        }

        const fs::path textures_path = path / textures_folder_name;
        if (!exists(textures_path))
        {
            log::error("Voxel textures folder path at '{}' does not exist.", textures_path.string());
            return false;
        }

        const auto block_models = load_block_models(block_models_path);
        if (!block_models)
        {
            log::error("Failed to import block models.");
            return false;
        }

        const auto block_types_and_texture_ids = load_block_types(block_types_path, *block_models);
        if (!block_types_and_texture_ids)
        {
            log::error("Failed to import block types.");
            return false;
        }

        m_path = path;
        m_block_models = *block_models;
        m_block_types = block_types_and_texture_ids->block_types;
        m_texture_ids = block_types_and_texture_ids->texture_ids;

        return true;
    }

    static void process_triangle_vertices(
        const std::vector<std::array<u32, 5>>& in_vertices,
        u32 face_index,
        u32 triangle_index,
        std::vector<BlockVertex>& out_processed_vertices)
    {
        const auto& v0 = in_vertices[triangle_index * 3 + 0];
        const auto& v1 = in_vertices[triangle_index * 3 + 1];
        const auto& v2 = in_vertices[triangle_index * 3 + 2];

        const v3i p0{v0[0], v0[1], v0[2]};
        const v3i p1{v1[0], v1[1], v1[2]};
        const v3i p2{v2[0], v2[1], v2[2]};

        // Pack normal vector
        const v3 p0_to_p1 = p1 - p0;
        const v3 p0_to_p2 = p2 - p0;
        const v3 normal = glm::normalize(glm::cross(p0_to_p1, p0_to_p2));
        const f32 n_pitch = std::asin(normal.y);
        const f32 n_yaw = std::atan2(normal.x, normal.z);

        // Remap from 0 to 1 to integer values depending on their number of bits
        const f32 normalized_pitch = (n_pitch + glm::half_pi<f32>()) / glm::pi<f32>();
        const f32 normalized_yaw = (n_yaw + glm::pi<f32>()) / glm::two_pi<f32>();

        const u32 packed_n_pitch = std::lround(normalized_pitch * voxel_constants::packed_pitch_max_value);
        const u32 packed_n_yaw = std::lround(normalized_yaw * voxel_constants::packed_yaw_max_value);

        for (i32 vertex_index = 0; vertex_index < 3; vertex_index++)
        {
            const auto& vertex = in_vertices[triangle_index * 3 + vertex_index];
            out_processed_vertices.push_back(
                BlockVertex
                    {
                        .x = vertex[0],
                        .y = vertex[1],
                        .z = vertex[2],
                        .u = vertex[3],
                        .v = vertex[4],
                        .tex_idx = face_index,
                        .n_pitch = packed_n_pitch,
                        .n_yaw = packed_n_yaw,
                    }
            );
        }
    }

    static std::optional<BlockModel> load_block_model(const YAML::Node& block_model_yml)
    {
        u32 face_index = 0;

        BlockModel model{};
        model.name = block_model_yml["name"].as<std::string>();

        for (const auto face_yml: block_model_yml["faces"])
        {
            const auto vertices = face_yml["vertices"].as<std::vector<std::array<u32, 5>>>();
            if (vertices.size() % 3 != 0)
            {
                log::warn("Number of vertices for faces in block model '{}' must be a multiple of 3.", model.name);
                return std::nullopt;
            }

            std::vector<BlockVertex> face_vertices{};
            face_vertices.reserve(vertices.size());

            const size_t triangle_count = vertices.size() / 3;
            for (size_t triangle_index = 0; triangle_index < triangle_count; triangle_index++)
                process_triangle_vertices(vertices, face_index, triangle_index, face_vertices);

            if (const auto occluded_by_yml = face_yml["occluded_by"])
            {
                // This face can be occluded
                const auto occluder_name = occluded_by_yml.as<std::string>();
                const auto occluder = magic_enum::enum_cast<voxel::Direction::Type>(occluder_name);

                if (!occluder)
                {
                    log::error("Failed to import block model '{}'; invalid occluder name: '{}'", model.name, occluder_name);
                    continue;
                }

                const auto dir_idx = magic_enum::enum_index(*occluder);
                assert(dir_idx);

                model.occluded_faces_per_side[*dir_idx].emplace_back(std::move(face_vertices));
            }
            else
            {
                model.unoccluded_faces.emplace_back(std::move(face_vertices));
            }

            face_index++;
        }

        return model;
    }

    std::optional<BlockModelList> VoxelPack::load_block_models(const fs::path& path)
    {
        BlockModelList result{};

        try
        {
            const auto root_yml = YAML::LoadFile(path.string());
            const auto block_models_yml = root_yml["block_models"];
            for (const auto block_model_yml: block_models_yml)
            {
                auto model = load_block_model(block_model_yml);
                if (!model)
                {
                    log::warn("Failed to import block model. Ignoring.");
                    continue;
                }

                // Check if result already contains a model with the same name
                const auto it = std::find_if(result.begin(), result.end(),
                    [&](const BlockModel& other)
                    {
                        return model->name == other.name;
                    }
                );
                if (it != result.end())
                {
                    log::warn("Multiple block models found with name: '{}'. Ignoring second.", model->name);
                    continue;
                }

                model->id = result.size();
                result.push_back(*model);
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block models: {}", e.what());
            return std::nullopt;
        }

        return result;
    }

    std::optional<VoxelPack::BlockTypeLoadResult> VoxelPack::load_block_types(const fs::path& path, const BlockModelList& block_models)
    {
        auto& voxel_module = g_engine->get_module_checked<VoxelModule>();

        u32 current_tex_index = 0;

        BlockTypeLoadResult result{};
        auto& [block_types, texture_index_map] = result;

        try
        {
            const auto root_yml = YAML::LoadFile(path.string());
            const auto block_types_yml = root_yml["block_types"];
            for (const auto block_type_yml : block_types_yml)
            {
                const auto id = block_type_yml["id"].as<BlockID>();
                if (id >= block_types.size())
                    block_types.resize(id + 1);

                const auto name = block_type_yml["name"].as<std::string>();

                if (id == 0)
                {
                    log::warn("Block type with name '{}' uses reserved id: {}. Skipping.", name, id);
                    continue;
                }

                // Find preset id
                const auto preset_name = block_type_yml["preset"].as<std::string>();
                const auto preset_id = voxel_module.find_block_preset_id(preset_name);
                if (!preset_id)
                {
                    log::warn("Block type with name '{}' uses unknown preset: '{}'. Skipping.", name, preset_name);
                    continue;
                }

                // Find model name
                const auto model_name = block_type_yml["model"].as<std::string>();
                const auto model_it = std::find_if(block_models.begin(), block_models.end(),
                    [&](const auto& item)
                    {
                        return item.name == model_name;
                    }
                );
                if (model_it == block_models.end())
                {
                    log::warn("Block type with name '{}' uses unknown model with name: '{}'. Skipping.", name, model_name);
                    continue;
                }

                // Find texture ids
                const auto texture_names = block_type_yml["textures"].as<std::vector<std::string>>();

                u32 face_count = model_it->calculate_face_count();
                if (face_count != texture_names.size())
                {
                    log::warn("Mismatch between number of faces in model and number of textures for block type '{}'. Skipping.", name);
                    continue;
                }

                std::vector<u32> texture_ids{};
                texture_ids.reserve(texture_names.size());

                for (const auto& texture_name : texture_names)
                {
                    const auto tex_it = texture_index_map.find(texture_name);
                    if (tex_it == texture_index_map.end())
                    {
                        texture_ids.push_back(current_tex_index);
                        texture_index_map.insert({ texture_name, current_tex_index++ });
                    }
                    else
                    {
                        texture_ids.push_back(tex_it->second);
                    }
                }

                block_types[id] =
                    BlockType
                    {
                        .name = name,
                        .block_id = id,
                        .texture_ids = texture_ids,
                        .model_id = model_it->id,
                        .preset_id = *preset_id,
                    };
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block types: {}", e.what());
            return std::nullopt;
        }

        return result;
    }
}