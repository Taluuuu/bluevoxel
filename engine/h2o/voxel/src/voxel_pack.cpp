#include "voxel/voxel_pack.h"

#include "core/engine.h"
#include "voxel/voxel_module.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    void VoxelPack::edit_block_type(BlockID block_id, BlockType& edited_block_type)
    {
        // Do some validations here
        assert(block_id < m_block_types.size());
        assert(edited_block_type.model_id < m_block_models.size());

        // TODO: Test this with 0 textures
        const auto& block_model = m_uncooked_block_models[edited_block_type.model_id];
        edited_block_type.texture_ids.resize(block_model.face_count(), 0);

        m_block_types[block_id] = edited_block_type;

        on_voxel_pack_updated.broadcast({ *this });
    }

    BlockID VoxelPack::create_block_type(const std::string& name)
    {
        const auto it = std::find(m_block_types.begin(), m_block_types.end(), std::nullopt);

        const BlockID block_id = it - m_block_types.begin();
        if (it == m_block_types.end())
        {
            m_block_types.emplace_back(BlockType{ name, block_id, {}, 0, 0 });
        }
        else
        {
            *it = BlockType{ name, block_id, {}, 0, 0 };
        }

        on_voxel_pack_updated.broadcast({ *this });

        return block_id;
    }

    void VoxelPack::delete_block_type(BlockID block_id)
    {
        if (block_id < m_block_types.size())
            m_block_types[block_id] = std::nullopt;

        // Trim null blocks at the end
        i32 i = i32(m_block_types.size()) - 1;
        for (; i >= 0; i--)
        {
            if (m_block_types[i])
                break;
        }
        m_block_types.resize(i + 1);

        on_voxel_pack_updated.broadcast({ *this });
    }

    const UncookedBlockModel* VoxelPack::get_uncooked_block_model(u32 model_id) const
    {
        if (model_id < m_uncooked_block_models.size())
            return &m_uncooked_block_models[model_id];

        return nullptr;
    }

    UncookedBlockModel* VoxelPack::get_uncooked_block_model(u32 model_id)
    {
        if (model_id < m_uncooked_block_models.size())
            return &m_uncooked_block_models[model_id];

        return nullptr;
    }

    void VoxelPack::build_block_model(u32 model_id)
    {
        assert(m_uncooked_block_models.size() == m_block_models.size());

        if (model_id < m_uncooked_block_models.size())
        {
            auto& uncooked_model = m_uncooked_block_models[model_id];

            m_block_models[model_id] = uncooked_model.build();

            on_voxel_pack_updated.broadcast({ *this });
        }
    }

    void VoxelPack::for_each_uncooked_block_model(const std::function<void(const UncookedBlockModel&)>& function) const
    {
        for (const auto& model : m_uncooked_block_models)
            function(model);
    }

    u32 VoxelPack::create_block_model(const std::string& name)
    {
        const u32 model_id = m_uncooked_block_models.size();
        auto& new_model = m_uncooked_block_models.emplace_back();
        new_model.name = name;
        new_model.id = model_id;

        m_block_models.emplace_back(new_model.build());
        on_voxel_pack_updated.broadcast({ *this });

        return model_id;
    }

    void VoxelPack::add_face_to_model(u32 model_id, const UncookedBlockModel::Face& face)
    {
        if (model_id < m_uncooked_block_models.size())
        {
            m_uncooked_block_models[model_id].add_face(face);

            for (auto& block_type : m_block_types)
            {
                if (block_type && block_type->model_id == model_id)
                    block_type->texture_ids.push_back(0);
            }

            on_voxel_pack_updated.broadcast({ *this });
        }
    }

    void VoxelPack::remove_face_from_model(u32 model_id, UncookedBlockModel::FaceHandle face_handle)
    {
        if (model_id < m_uncooked_block_models.size())
        {
            auto& block_model = m_uncooked_block_models[model_id];
            block_model.delete_face(face_handle);
            m_block_models[model_id] = block_model.build();

            for (auto& block_type : m_block_types)
            {
                if (!block_type || block_type->model_id != model_id)
                    continue;

                block_type->texture_ids.erase(block_type->texture_ids.cbegin() + face_handle.face_index);
            }

            on_voxel_pack_updated.broadcast({ *this });
        }
    }

    const BlockModel* VoxelPack::get_block_model(u32 model_id) const
    {
        if (model_id < m_block_models.size())
            return &m_block_models[model_id];

        return nullptr;
    }

    void VoxelPack::save() const
    {
        try
        {
            save_block_types();
            save_block_models();
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

        const auto uncooked_block_models = load_block_models(block_models_path);
        if (!uncooked_block_models)
            return false;

        const auto texture_ids = generate_texture_ids(textures_path);
        const auto block_types = load_block_types(block_types_path, *uncooked_block_models, texture_ids);
        if (!block_types)
            return false;

        m_uncooked_block_models = *uncooked_block_models;
        m_block_models.clear();
        for (const auto& uncooked_model : m_uncooked_block_models)
            m_block_models.push_back(uncooked_model.build());

        m_path = path;
        m_block_types = *block_types;
        m_texture_ids = texture_ids;

        return true;
    }

    TextureNameIdMap VoxelPack::generate_texture_ids(const fs::path& path)
    {
        TextureNameIdMap result{};
        u32 current_texture_id = 0;
        for (const auto& texture_path : fs::directory_iterator(path))
        {
            if (!texture_path.is_directory())
                result.insert({ texture_path.path().filename().string(), current_texture_id++ });
        }

        return result;
    }

    std::optional<UncookedBlockModelList> VoxelPack::load_block_models(const fs::path& path)
    {
        UncookedBlockModelList result{};

        UncookedBlockModel null_model{};
        null_model.name = "none";
        null_model.id = 0;
        result.push_back(null_model);

        try
        {
            const auto root_yml = YAML::LoadFile(path.string());
            const auto block_models_yml = root_yml["block_models"];
            for (const auto block_model_yml: block_models_yml)
            {
                auto model = block_model_yml.as<UncookedBlockModel>();
                model.id = result.size();

                // Check if result already contains a model with the same name
                const auto it = std::find_if(result.begin(), result.end(),
                    [&](const UncookedBlockModel& other)
                    { return model.name == other.name; }
                );
                if (it != result.end())
                {
                    log::warn("Multiple block models found with name: '{}'. Ignoring second.", model.name);
                    continue;
                }

                result.push_back(model);
            }
        }
        catch (const std::exception& e)
        {
            log::error("Failed to import block models: {}", e.what());
            return std::nullopt;
        }

        return result;
    }

    std::optional<BlockTypeList> VoxelPack::load_block_types(
        const fs::path& path, const UncookedBlockModelList& block_models, const TextureNameIdMap& texture_id_map)
    {
        auto& voxel_module = g_engine->get_module_checked<VoxelModule>();

        BlockTypeList result{};
        result.emplace_back(BlockType{ "air", 0, {}, 0, 0, true });

        try
        {
            const auto root_yml = YAML::LoadFile(path.string());
            const auto block_types_yml = root_yml["block_types"];
            for (const auto block_type_yml : block_types_yml)
            {
                const auto id = block_type_yml["id"].as<BlockID>();
                if (id >= result.size())
                    result.resize(id + 1);

                const auto name = block_type_yml["name"].as<std::string>();
                if (const auto& block_type_with_id = result[id])
                {
                    log::warn("Block type with name '{}' uses already assigned id: {} to block '{}'.", name, id, block_type_with_id->name);
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
                    { return item.name == model_name; }
                );
                if (model_it == block_models.end())
                {
                    log::warn("Block type with name '{}' uses unknown model with name: '{}'. Skipping.", name, model_name);
                    continue;
                }

                const bool is_transparent = block_type_yml["is_transparent"].as<bool>();

                // Find texture ids
                const auto texture_names = block_type_yml["textures"].as<std::vector<std::string>>();

                u32 face_count = model_it->face_count();
                if (face_count != texture_names.size())
                {
                    log::warn("Mismatch between number of faces in model and number of textures for block type '{}'. Skipping.", name);
                    continue;
                }

                std::vector<u32> texture_ids{};
                texture_ids.reserve(texture_names.size());

                for (const auto& texture_name : texture_names)
                {
                    const auto tex_it = texture_id_map.find(texture_name);
                    if (tex_it == texture_id_map.end())
                    {
                        log::warn("Block type '{}' uses unknown texture '{}'.", name, texture_name);
                        texture_ids.push_back(0);
                    }
                    else
                    {
                        texture_ids.push_back(tex_it->second);
                    }
                }

                result[id] =
                    BlockType
                    {
                        .name = name,
                        .block_id = id,
                        .texture_ids = texture_ids,
                        .model_id = model_it->id,
                        .preset_id = *preset_id,
                        .is_transparent = is_transparent,
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

    void VoxelPack::save_block_types() const
    {
        YAML::Emitter yaml{};

        yaml << YAML::BeginMap;

        yaml << YAML::Key << "block_types";
        yaml << YAML::Value;

        yaml << YAML::BeginSeq;

        for (const auto& block_type : m_block_types)
        {
            if (!block_type || block_type->block_id == 0)
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

    void VoxelPack::save_block_models() const
    {
        YAML::Emitter yaml{};

        yaml << YAML::BeginMap;

        yaml << YAML::Key << "block_models";
        yaml << YAML::Value;

        yaml << YAML::BeginSeq;

        for (const auto& block_model : m_uncooked_block_models)
        {
            if (!block_model.id)
                continue;

            yaml << YAML::BeginMap;

            yaml << YAML::Key << "name";
            yaml << YAML::Value << block_model.name;

            yaml << YAML::Key << "faces";
            yaml << YAML::Value;

            yaml << YAML::BeginSeq;

            block_model.for_each_face(
                [&](UncookedBlockModel::FaceHandle face_handle, const UncookedBlockModel::Face& face)
                {
                    yaml << YAML::BeginMap;

                    yaml << YAML::Key << "triangles";
                    yaml << YAML::Value;

                    yaml << YAML::BeginSeq;

                    for (const auto& triangle : face)
                    {
                        yaml << YAML::Flow << YAML::BeginSeq;

                        for (const auto& vertex : triangle.vertices)
                        {
                            yaml << YAML::Flow << YAML::BeginSeq;

                            yaml << vertex.position.x;
                            yaml << vertex.position.y;
                            yaml << vertex.position.z;
                            yaml << vertex.uv.x;
                            yaml << vertex.uv.y;

                            yaml << YAML::EndSeq;
                        }

                        yaml << YAML::EndSeq;
                    }

                    yaml << YAML::EndSeq;

                    yaml << YAML::EndMap;
                }
            );

            yaml << YAML::EndSeq;

            yaml << YAML::EndMap;
        }

        yaml << YAML::EndSeq;

        yaml << YAML::EndMap;

        const auto block_models_path = m_path / block_models_file_name;
        std::ofstream file(block_models_path.string());
        file << yaml.c_str();

        log::info("Saved block models to file at '{}'", absolute(block_models_path).string());
    }
}