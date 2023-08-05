#pragma once

#include "core/handle_types.h"
#include "voxel_constants.h"

#include <array>
#include <memory>

namespace h2o
{
    class Chunk;
    using ChunkColumn = std::array<Chunk, voxel_constants::vertical_chunk_count>;

    /**
     * A weak handle to a specific chunk in a column.
     */
    struct ChunkWeakHandle
    {
        WeakHandle<ChunkColumn> chunk_column;
        i32 height;

        [[nodiscard]] Chunk* chunk() const;
        [[nodiscard]] bool operator==(const ChunkWeakHandle& other) const;
    };
}