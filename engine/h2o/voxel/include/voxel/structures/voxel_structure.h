#pragma once

#include "voxel/block.h"

#include <optional>
#include <vector>

namespace h2o
{
    class VoxelStructure
    {
    public:

        explicit VoxelStructure(v3i size = { 1, 1, 1 });

        void set_block(const v3i& pos, Block block);
        [[nodiscard]] Block get_block(const v3i& pos) const;

        void resize(const v3i& new_size);
        [[nodiscard]] const v3i& size() const { return m_data.size; }

        struct StructureData
        {
            explicit StructureData(v3i size = { 0, 0, 0 });

            v3i size{};
            std::vector<Block> blocks{};

            void set_block(const v3i& pos, Block block);
            [[nodiscard]] Block get_block(const v3i& pos) const;

        private:

            [[nodiscard]] bool is_valid_pos(const v3i& pos) const;
            [[nodiscard]] std::optional<size_t> to_index(const v3i& pos) const;

        };

    private:

        StructureData m_data{};

    };
}