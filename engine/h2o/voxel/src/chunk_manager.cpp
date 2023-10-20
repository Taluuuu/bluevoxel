#include "voxel/chunk_manager_interface.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    void IChunkManager::fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    {
        fetch_chunk_column({ chunk_pos.x, chunk_pos.z },
            [&](ChunkColumn* chunk_column)
            {
                if (chunk_column)
                {
                    function(chunk_column->get_chunk_safe(chunk_pos.y));
                }
                else
                {
                    function(nullptr);
                }
            }
        );
    }

    void IChunkManager::fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const
    {
        fetch_chunk_column({ chunk_pos.x, chunk_pos.z },
            [&](const ChunkColumn* chunk_column)
            {
                if (chunk_column)
                {
                    function(chunk_column->get_chunk_safe(chunk_pos.y));
                }
                else
                {
                    function(nullptr);
                }
            }
        );
    }
}