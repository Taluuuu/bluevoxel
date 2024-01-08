#pragma once

#include "block_model.h"
#include "block_type.h"
#include "core/resources.h"

#include <optional>
#include <string_view>
#include <vector>

namespace h2o
{
    using BlockModelList = std::vector<BlockModel>;
    using BlockTypeList = std::vector< std::optional<BlockType> >;
    using TextureNameIdMap = std::unordered_map<std::string, u32>;

    class VoxelPack : public IResource
    {
    public:

        VoxelPack() = default;
        ~VoxelPack() override = default;

        [[nodiscard]] const BlockModelList&   block_models() const { return m_block_models; }
        [[nodiscard]] const BlockTypeList&    block_types()  const { return m_block_types;  }
        [[nodiscard]] const TextureNameIdMap& texture_ids()  const { return m_texture_ids;  }

        [[nodiscard]] const fs::path& path() const { return m_path; }

        // Apply local changes to files in the specified path
        void save(const fs::path& path) const;

        // IResource interface
        bool load(const fs::path& path) override;

    public:

        static constexpr std::string_view block_types_file_name { "block_types.yml" };
        static constexpr std::string_view block_models_file_name { "block_models.yml" };
        static constexpr std::string_view textures_folder_name { "textures" };

    protected:

        static std::optional<BlockModelList> load_block_models(const fs::path& path);

        struct BlockTypeLoadResult
        {
            BlockTypeList block_types{};
            TextureNameIdMap texture_ids{};
        };
        static std::optional<BlockTypeLoadResult> load_block_types(const fs::path& path, const BlockModelList& block_models);

    private:

        BlockTypeList m_block_types{};
        BlockModelList m_block_models{};
        TextureNameIdMap m_texture_ids{};

        fs::path m_path{};

    };
}