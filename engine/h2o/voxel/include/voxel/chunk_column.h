#pragma once

#include "chunk.h"

#include <mutex>

namespace h2o
{
    // using ChunkColumn = std::array< Chunk, voxel_constants::vertical_chunk_count >;

    // class ChunkColumn
    // {
    // public:
    //
    //     explicit ChunkColumn(v2i chunk_col_pos);
    //
    //     void tick();
    //
    //     [[nodiscard]] v2i chunk_column_pos() const
    //     { return m_chunk_col_pos; }
    //
    //     // Generation stage
    //     [[nodiscard]] i32 generation_stage() const
    //     { return m_generation_stage; }
    //
    //     [[nodiscard]] bool is_generated() const
    //     { return generation_stage() == voxel_constants::max_generation_stage; }
    //
    //     void increment_generation_stage();
    //
    //     void finish_generation();
    //
    //     [[nodiscard]] Chunk& operator[](size_t index)
    //     {
    //         assert(index < m_chunks.size());
    //         assert(m_chunks[index]);
    //         return *m_chunks[index];
    //     }
    //
    //     [[nodiscard]] const Chunk& operator[](size_t index) const
    //     {
    //         assert(index < m_chunks.size());
    //         assert(m_chunks[index]);
    //         return *m_chunks[index];
    //     }
    //
    //     [[nodiscard]] Chunk* get_chunk_safe(i32 y);
    //
    //     [[nodiscard]] const Chunk* get_chunk_safe(i32 y) const;
    //
    //     [[nodiscard]] std::mutex& mutex() const { return m_mutex; }
    //
    // public:
    //
    //     class iterator
    //     {
    //     public:
    //         using value_type = Chunk;
    //         using pointer = Chunk*;
    //         using reference = Chunk&;
    //
    //         iterator(ChunkColumn& column, size_t index)
    //             : m_column(column), m_index(index) {}
    //
    //         reference operator*() const
    //         {
    //             return m_column[m_index];
    //         }
    //
    //         pointer operator->() const
    //         {
    //             return &m_column[m_index];
    //         }
    //
    //         iterator& operator++()
    //         {
    //             ++m_index;
    //             return *this;
    //         }
    //
    //         iterator operator++(int)
    //         {
    //             iterator temp = *this;
    //             ++(*this);
    //             return temp;
    //         }
    //
    //         bool operator==(const iterator& other) const
    //         {
    //             return m_index == other.m_index;
    //         }
    //
    //         bool operator!=(const iterator& other) const
    //         {
    //             return !(*this == other);
    //         }
    //
    //     private:
    //
    //         ChunkColumn& m_column;
    //         size_t m_index;
    //
    //     };
    //
    //     iterator begin()
    //     {
    //         return {*this, 0};
    //     }
    //
    //     iterator end()
    //     {
    //         return {*this, voxel_constants::vertical_chunk_count};
    //     }
    //
    // private:
    //
    //     std::array< std::shared_ptr<Chunk>, voxel_constants::vertical_chunk_count > m_chunks{};
    //
    //     i32 m_generation_stage { 0 };
    //
    //     const v2i m_chunk_col_pos { 0, 0 };
    //
    //     mutable std::mutex m_mutex;
    //
    // };
}