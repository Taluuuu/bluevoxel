#pragma once

#include "core/types.h"

#include <functional>
#include <memory>

namespace h2o
{
    class IChunkManager;
    class ChunkColumn;

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(v2i center);

        [[nodiscard]] ChunkColumn* center_chunk() const;
        void for_each_chunk_column(const std::function<void(ChunkColumn*)>& function) const;

        void set_chunk_column_at(v2i chunk_column_pos, ChunkColumn* chunk_column);

    private:

        [[nodiscard]] bool in_range(v2i local_chunk_column_pos) const;
        [[nodiscard]] size_t to_index(v2i local_chunk_column_pos) const;

    private:

        std::array<ChunkColumn*, 9> m_chunks{};
        v2i m_center{};

    };
}