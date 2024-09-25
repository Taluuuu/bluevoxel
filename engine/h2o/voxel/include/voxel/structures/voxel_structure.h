#pragma once

#include "voxel/block.h"

#include <optional>
#include <vector>

namespace h2o
{
    class VoxelStructure
    {
    public:

        explicit VoxelStructure(const std::string& name, v3i size = { 1, 1, 1 });

        void set_block(const v3i& pos, Block block);
        [[nodiscard]] Block get_block(const v3i& pos) const;

        void resize(const v3i& new_size);
        [[nodiscard]] const v3i& size() const { return m_data.size; }

        // Moves the structure by leaving blank blocks.
        // Cannot delete blocks.
        // Returns true on success.
        bool move(const v3i& delta);

        void set_name(const std::string& name) { m_name = name; }
        [[nodiscard]] const std::string& name() const { return m_name; }

        void clear();

    private:

        // This struct exists to allow for easier resizing, probably in
        // editor mode
        struct StructureData
        {
            explicit StructureData(v3i size = { 0, 0, 0 });

            v3i size{};
            std::vector<Block> blocks{};

            // Returns true if in bounds
            bool set_block(const v3i& pos, Block block);
            [[nodiscard]] Block get_block(const v3i& pos) const;

        private:

            [[nodiscard]] bool is_valid_pos(const v3i& pos) const;
            [[nodiscard]] std::optional<size_t> to_index(const v3i& pos) const;

        };

        StructureData m_data{};
        std::string m_name{};

    };
}
