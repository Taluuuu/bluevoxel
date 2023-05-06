#pragma once

#include "core/handle_types.h"
#include "core/types.h"
#include "voxel/chunk.h" // Remove once I can get by with just a forward declaration

#include <vector>

namespace h2o
{
    class ChunkMgr
    {
    public:

        Chunk* get_chunk_at(const v3i& pos) const;
        const std::vector<Chunk*>& get_chunks() const;

    private:

        static size_t to_index(const v3i& pos);
        static size_t to_pos(size_t index);

    private:

        std::vector<Chunk> m_chunks;

    };
}