#pragma once

#include "core/types.h"
#include "block.h"

#include <cstddef>
#include <iterator>
#include <vector>

namespace h2o
{
    template<size_t Size = 32>
    class Chunk
    {
    public:

        explicit Chunk();

        [[nodiscard]] size_t volume() const { return Size * Size * Size; }
        [[nodiscard]] size_t size() const { return Size; }

    private:

        std::vector<Block> m_blocks;

    };
}