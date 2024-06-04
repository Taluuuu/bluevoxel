#pragma once

#include "block.h"

#include <vector>

namespace h2o
{
    class VoxelStructure
    {
    public:

        VoxelStructure();

        [[nodiscard]] Block get_block(const v3i& pos) const;
        [[nodiscard]] bool is_valid_pos(const v3i& pos) const;

        void resize(const v3i& new_size);
        void set_block(const v3i& pos, Block block);

    private:

        [[nodiscard]] size_t to_index(const v3i& pos) const;

    private:

        v3i m_size{};
        std::vector<Block> m_blocks{};

    };
}