#pragma once

#include "core/types.h"
#include "block.h"

#include <cstddef>
#include <iterator>
#include <vector>

namespace h2o
{
    class Chunk
    {
    public:

        explicit Chunk(size_t size);

        [[nodiscard]] size_t volume() const { return m_size * m_size * m_size; }

    private:

        std::vector<Block> m_blocks;

        size_t m_size;

    };
}