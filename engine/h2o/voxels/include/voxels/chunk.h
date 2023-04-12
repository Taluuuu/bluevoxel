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

        struct Iterator
        {
            using iterator_category = std::input_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = Block;
            using pointer           = Block*;
            using reference         = Block&;

            Iterator(pointer ptr) : m_ptr(ptr) {}

            reference operator*() const { return *m_ptr; }
            pointer operator->() { return m_ptr; }

            // Prefix increment
            Iterator& operator++() { m_ptr++; return *this; }

            // Postfix increment
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

        private:

            pointer m_ptr;

        };

    private:

        std::vector<Block> m_blocks;

        size_t m_size;

    };
}