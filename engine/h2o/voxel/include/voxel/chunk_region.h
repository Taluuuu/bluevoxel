#pragma once

#include "core/types.h"
#include "voxel/block.h"

#include <optional>
#include <vector>

namespace h2o
{
    class Chunk;

    // Useless on its own, meant to be created by a chunk manager.
    class ChunkRegion
    {
    public:

        ChunkRegion(const v3i& min, const v3i& size);

        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos, const v3i& relative_to_chunk_pos = { 0, 0, 0 }) const;
        bool set_block_at(const v3i& block_pos, Block block, const v3i& relative_to_chunk_pos = { 0, 0, 0 });

        // Not meant to be accessed by users...
        void add_chunk(Chunk& chunk);
        void lock_chunks(bool exclusive);
        void unlock_chunks(bool exclusive);

    private:

        [[nodiscard]] Chunk* get_chunk_at(const v3i& relative_chunk_pos, const v3i& relative_to) const;

        [[nodiscard]] bool in_range(const v3i& local_chunk_pos) const;
        [[nodiscard]] size_t to_index(const v3i& local_chunk_pos) const;

    private:

        // 3D vector of chunk pointers
        std::vector<Chunk*> m_chunks{};

        v3i m_min{};
        v3i m_size{};

    };
}