#pragma once

#include "chunk_types.h"
#include "core/handle_types.h"
#include "core/types.h"

#include <functional>
#include <memory>
#include <optional>

namespace h2o
{
    class ChunkSystem;

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(const WeakHandle<ChunkSystem>& chunk_system);
        virtual ~ChunkRegion() = default;

        void set_size(u32 new_size);
        void set_corner_pos(v2i new_corner_pos);
        void update_data(v2i new_corner_pos, u32 new_size);

        void for_each_chunk(const std::function<void(Chunk&)>& fun) const;
        [[nodiscard]] Chunk* get_chunk_at(const v3i& chunk_pos) const;
        [[nodiscard]] WeakHandle<ChunkColumn> get_chunk_col_at(v2i chunk_col_pos) const;

        [[nodiscard]] v2i corner_pos() const { return m_corner_pos; }
        [[nodiscard]] v2i center_pos() const { return m_corner_pos + v2i{ size() / 2, size() / 2 }; }
        [[nodiscard]] u32 size() const { return m_size; }
        [[nodiscard]] bool in_region_bounds(const v3i& chunk_pos) const;
        [[nodiscard]] bool in_region_bounds(v2i chunk_pos) const;

    protected:

        // Override to run movement logic when the chunk region changes size
        // or moves. This function is also called on init.
        virtual void on_indices_changed(const std::vector<i32>& new_to_old_indices) {}
        virtual void on_chunk_fetched(const WeakHandle<ChunkColumn>& chunk_col, v2i local_chunk_pos) {}

        [[nodiscard]] std::optional<v3i> to_local_chunk_pos_3d(const v3i& chunk_pos) const;
        [[nodiscard]] std::optional<v2i> to_local_chunk_pos_2d(v2i chunk_pos) const;

        // Make an index into an array of size m_size * m_size
        [[nodiscard]] constexpr size_t to_index(v2i pos) const;

    private:

        // Generate an array of size new_size * new_size that contains all the
        // indices into the old array of chunks to move. If a given chunk is
        // in the new array but not in the old one, an index of -1 is placed.
        [[nodiscard]] std::vector<i32> gen_new_to_old_indices(v2i new_corner_pos, u32 new_size) const;

    private:

        WeakHandle<ChunkSystem> m_chunk_system = nullptr;

        std::vector< WeakHandle<ChunkColumn> > m_chunks_in_region;

        v2i m_corner_pos { 0, 0 };
        u32 m_size { 0 };

    };
}