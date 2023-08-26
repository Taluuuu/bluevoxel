#include "voxel/chunk_weak_handle.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    Chunk* ChunkWeakHandle::chunk() const
    {
        if (!chunk_column)
            return nullptr;

        return &(*chunk_column)[height];
    }

    bool ChunkWeakHandle::operator==(const ChunkWeakHandle& other) const
    {
        return
            chunk_column == other.chunk_column &&
            height       == other.height;
    }
}