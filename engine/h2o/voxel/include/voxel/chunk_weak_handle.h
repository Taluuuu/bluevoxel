#pragma once

#include "core/handle_types.h"
#include "core/types.h"

namespace h2o
{
    class Chunk;
    class ChunkColumn;

    /**
     * A weak handle to a specific chunk in a column.
     */
    struct ChunkWeakHandle
    {
        WeakHandle<ChunkColumn> chunk_column;
        i32 height;

        ChunkWeakHandle()
            : ChunkWeakHandle(nullptr) {}

        ChunkWeakHandle(std::nullptr_t)
            : chunk_column(nullptr), height(0) {}

        ChunkWeakHandle(const WeakHandle<ChunkColumn>& chunk_column, i32 height)
            : chunk_column(chunk_column), height(height) {}

        [[nodiscard]] Chunk* chunk() const;
        [[nodiscard]] bool operator==(const ChunkWeakHandle& other) const;
        [[nodiscard]] bool operator==(std::nullptr_t) const { return chunk_column == nullptr; }
        [[nodiscard]] operator bool() const { return chunk_column != nullptr; }
        [[nodiscard]] operator Chunk*() const { return chunk(); }
        [[nodiscard]] Chunk* operator->() const { return chunk(); }
    };
}