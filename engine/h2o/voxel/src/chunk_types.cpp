#include "voxel/chunk_types.h"

#include "voxel/chunk.h"

namespace h2o
{
    Chunk* ChunkWeakHandle::chunk() const
    {
        if (!chunk_column)
            return nullptr;

        assert(!chunk_column->empty());

        return &(*chunk_column)[height];
    }

    bool ChunkWeakHandle::operator==(const ChunkWeakHandle& other) const
    {
        return
            chunk_column == other.chunk_column &&
            height == other.height;
    }
}