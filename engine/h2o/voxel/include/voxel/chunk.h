#pragma once

#include "block.h"
#include "core/handle_types.h"
#include "core/types.h"
#include "voxel/voxel_constants.h"

#include <iterator>
#include <vector>

namespace h2o
{
    class ChunkSystem;

    class Chunk
    {
    public:

        Chunk() = default;

        void init();

        [[nodiscard]] Block get_block_at(const v3i& local_pos) const;
        [[nodiscard]] Block* get_block_ptr_at(const v3i& local_pos);
        void set_block_at(const v3i& local_pos, Block block);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] bool is_empty() const { return m_is_empty; }

        static constexpr bool is_valid_pos(const v3i& local_pos)
        {
            return
                local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
                local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
                local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
        }

    private:

        [[nodiscard]] bool is_initialized() const { return !m_blocks.empty(); }

    private:

        friend class ChunkColumn;

        std::vector<Block> m_blocks;

        v3i m_chunk_pos{};

        bool m_is_empty = true;

    };

    class ChunkColumn
    {
    public:

        explicit ChunkColumn(v2i chunk_col_pos);

        void init();
        [[nodiscard]] bool is_initialized() const { return m_is_initialized; }

        [[nodiscard]] v2i chunk_column_pos() const { return m_chunk_col_pos; }

        // Generation stage
        [[nodiscard]] i32 generation_stage() const { return m_generation_stage; }
        [[nodiscard]] bool is_generated() const { return generation_stage() == voxel_constants::max_generation_stage; }
        void increment_generation_stage();
        void finish_generation();

        [[nodiscard]] Chunk& operator[](size_t index)
        {
            assert(index < m_chunks.size());
            return m_chunks[index];
        }

        [[nodiscard]] const Chunk& operator[](size_t index) const
        {
            assert(index < m_chunks.size());
            return m_chunks[index];
        }

    public:

        class iterator
        {
        public:
            using value_type = Chunk;
            using pointer = Chunk*;
            using reference = Chunk&;

            iterator(ChunkColumn& column, size_t index) : m_column(column), m_index(index) {}

            reference operator*() const
            {
                return m_column[m_index];
            }

            pointer operator->() const
            {
                return &m_column[m_index];
            }

            iterator& operator++()
            {
                ++m_index;
                return *this;
            }

            iterator operator++(int)
            {
                iterator temp = *this;
                ++(*this);
                return temp;
            }

            bool operator==(const iterator& other) const
            {
                return m_index == other.m_index;
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }

        private:

            ChunkColumn& m_column;
            size_t m_index;

        };

        iterator begin()
        {
            return { *this, 0 };
        }

        iterator end()
        {
            return { *this, voxel_constants::vertical_chunk_count };
        }

    private:

        std::array<Chunk, voxel_constants::vertical_chunk_count> m_chunks{};

        i32 m_generation_stage { 0 };

        v2i m_chunk_col_pos { 0, 0 };

        bool m_is_initialized { false };

    };

    /**
     * A weak handle to a specific chunk in a column.
     */
    struct ChunkWeakHandle
    {
        WeakHandle<ChunkColumn> chunk_column;
        i32 height;

        ChunkWeakHandle(std::nullptr_t)
            : chunk_column(nullptr), height(0) {}

        ChunkWeakHandle(const WeakHandle<ChunkColumn>& chunk_column, i32 height)
            : chunk_column(chunk_column), height(height) {}

        [[nodiscard]] Chunk* chunk() const;
        [[nodiscard]] bool operator==(const ChunkWeakHandle& other) const;
        [[nodiscard]] bool operator==(std::nullptr_t) const { return chunk_column == nullptr; }
    };
}