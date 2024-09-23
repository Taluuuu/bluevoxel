#pragma once

#include "block_type.h"
#include "core/events.h"
#include "core/resources.h"
#include "structures/voxel_structure_manager.h"
#include "voxel/block_model.h"
#include "voxel/uncooked_block_model.h"

#include <functional>
#include <optional>
#include <map>
#include <string_view>
#include <vector>

namespace h2o
{
    class UncookedBlockModel;
    class VoxelPack;

    using BlockModelList = std::vector<BlockModel>;
    using UncookedBlockModelList = std::vector<UncookedBlockModel>;
    using BlockTypeList = std::vector<std::optional<BlockType>>;
    using TextureNameIdMap = std::map<std::string, u32>;

    struct VoxelPackUpdatedEvent { const VoxelPack& voxel_pack; };

    class VoxelPack : public IResource
    {
    public:

        VoxelPack() = default;
        ~VoxelPack() override = default;

        [[nodiscard]] const BlockTypeList&    block_types()  const { return m_block_types;  }
        [[nodiscard]] const TextureNameIdMap& texture_ids()  const { return m_texture_ids;  }

        [[nodiscard]] const fs::path& path()     const { return m_path;     }

        // Block types
        [[nodiscard]] const BlockType* get_block_type(BlockID block_id) const;
        void edit_block_type(BlockID block_id, BlockType& edited_block_type);
        BlockID create_block_type(const std::string& name);
        void delete_block_type(BlockID block_id);

        // Uncooked block models
        [[nodiscard]] const UncookedBlockModel* get_uncooked_block_model(u32 model_id) const;
        [[nodiscard]] UncookedBlockModel* get_uncooked_block_model(u32 model_id);
        void build_block_model(u32 model_id);
        void for_each_uncooked_block_model(const std::function<void(const UncookedBlockModel&)>& function) const;
        u32 create_block_model(const std::string& name);
        void add_face_to_model(u32 model_id, const UncookedBlockModel::Face& face);
        void remove_face_from_model(u32 model_id, UncookedBlockModel::FaceHandle face_handle);

        // Block models
        // Only needs a getter; the only time we should access this is for rendering, as uncooked
        // block models are much easier to work with.
        [[nodiscard]] const BlockModel* get_block_model(u32 model_id) const;

        VoxelStructureManager& structure_manager() { return m_structure_manager; }
        const VoxelStructureManager& structure_manager() const { return m_structure_manager; }

        // Apply local changes
        void save() const;

        // IResource interface
        bool load(const fs::path& path) override;

    public:

        static constexpr std::string_view block_types_file_name  { "block_types.yml"  };
        static constexpr std::string_view block_models_file_name { "block_models.yml" };
        static constexpr std::string_view structures_file_name   { "structures.yml"   };
        static constexpr std::string_view textures_folder_name   { "textures"         };

        Event<VoxelPackUpdatedEvent> on_voxel_pack_updated{};

    protected:

        static TextureNameIdMap generate_texture_ids(const fs::path& path);
        static std::optional<UncookedBlockModelList> load_block_models(const fs::path& path);
        static std::optional<BlockTypeList> load_block_types(
            const fs::path& path, const UncookedBlockModelList& block_models, const TextureNameIdMap& texture_id_map);

        void save_block_types() const;
        void save_block_models() const;

    private:

        // There should always be a block model for every uncooked block model
        UncookedBlockModelList m_uncooked_block_models{};
        BlockModelList m_block_models{};

        BlockTypeList m_block_types{};
        TextureNameIdMap m_texture_ids{};

        VoxelStructureManager m_structure_manager{};

        fs::path m_path{};

    };
}
