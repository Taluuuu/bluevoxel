#pragma once

#include "core/events.h"
#include "core/handle_types.h"
#include "core/types.h"

#include <functional>
#include <memory>
#include <optional>

namespace h2o
{
    class Chunk;
    class ChunkColumn;
    class ChunkSystem;
    struct Block;
    struct ChunkWeakHandle;

    enum class BlockPositionSpace
    {
        RelativeToCorner,
        RelativeToCenterChunk,
        World
    };

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(ChunkSystem& chunk_system);
        virtual ~ChunkRegion() = default;

        void set_size(u32 new_size);
        void set_corner_pos(v2i new_corner_pos);
        void update_data(v2i new_corner_pos, u32 new_size);

        void for_each_chunk(const std::function<void(Chunk&)>& fun) const;
        [[nodiscard]] ChunkWeakHandle get_chunk_at(const v3i& chunk_pos) const;
        [[nodiscard]] WeakHandle<ChunkColumn> get_chunk_col_at(v2i chunk_col_pos) const;

        [[nodiscard]] v2i corner_pos() const { return m_corner_pos; }
        [[nodiscard]] v2i center_pos() const { return m_corner_pos + v2i{ size() / 2, size() / 2 }; }
        [[nodiscard]] u32 size() const { return m_size; }
        [[nodiscard]] bool in_region_bounds(const v3i& chunk_pos) const;
        [[nodiscard]] bool in_region_bounds(v2i chunk_pos) const;
        [[nodiscard]] ChunkSystem& chunk_system() const;

    protected:

        // Override to run movement logic when the chunk region changes size
        // or moves. This function is also called on init.
        virtual void on_indices_changed(const std::vector<i32>& new_to_old_indices) {}
        virtual void on_chunk_fetched(const WeakHandle<ChunkColumn>& chunk_col) {}

        [[nodiscard]] std::optional<v3i> to_local_chunk_pos_3d(const v3i& chunk_pos) const;
        [[nodiscard]] std::optional<v2i> to_local_chunk_pos_2d(v2i chunk_pos) const;

        // Make an index into an array of size m_size * m_size
        [[nodiscard]] constexpr size_t to_index(v2i pos) const
        { return pos.x * m_size + pos.y; }

    private:

        // Generate an array of size new_size * new_size that contains all the
        // indices into the old array of chunks to move. If a given chunk is
        // in the new array but not in the old one, an index of -1 is placed.
        [[nodiscard]] std::vector<i32> gen_new_to_old_indices(v2i new_corner_pos, u32 new_size) const;

    private:

        ChunkSystem* m_chunk_system = nullptr;
        EventHandle m_on_chunk_loaded_handle;

        std::vector< WeakHandle<ChunkColumn> > m_chunks_in_region;

        v2i m_corner_pos { 0, 0 };
        u32 m_size { 0 };

    };

    class StaticChunkRegion
    {
    public:

        StaticChunkRegion(v2i corner, i32 size, ChunkSystem& chunk_system);

        [[nodiscard]] i32 size() const { return m_size; }
        [[nodiscard]] v2i corner() const { return m_corner; }

        [[nodiscard]] WeakHandle<ChunkColumn> get_center_chunk() const;
        [[nodiscard]] WeakHandle<ChunkColumn> get_chunk_col_at(v2i chunk_col_pos) const;

        [[nodiscard]] Block* get_block_ptr_at(v3i block_pos, BlockPositionSpace block_pos_type) const;

        void for_each_chunk_column(const std::function<void(const WeakHandle<ChunkColumn>&)>& fun) const;

    public:

        class iterator
        {
        public:

            using value_type = WeakHandle<ChunkColumn>;
            using pointer = WeakHandle<ChunkColumn>*;
            using reference = WeakHandle<ChunkColumn>&;

            iterator(StaticChunkRegion& region, size_t index)
                : m_region(region), m_index(index) {}

            reference operator*() const
            { return m_region.m_chunks_in_region[m_index]; }

            pointer operator->() const
            { return &m_region.m_chunks_in_region[m_index]; }

            iterator& operator++()
            { ++m_index; return *this; }

            iterator operator++(int)
            { iterator temp = *this; ++(*this); return temp; }

            bool operator==(const iterator& other) const
            { return m_index == other.m_index; }

            bool operator!=(const iterator& other) const
            { return !(*this == other); }

        private:

            StaticChunkRegion& m_region;
            size_t m_index;

        };

        iterator begin()
        { return { *this, 0 }; }

        iterator end()
        { return { *this, m_chunks_in_region.size() }; }

    protected:

        [[nodiscard]] bool is_in_region(v2i chunk_col_pos) const;
        [[nodiscard]] std::optional<v2i> to_local_chunk_pos(v2i chunk_col_pos) const;
        [[nodiscard]] size_t to_index(v2i local_chunk_pos) const;

    private:

        i32 m_size;
        v2i m_corner;

        std::vector< WeakHandle<ChunkColumn> > m_chunks_in_region;

    };
}