#include "voxel/chunk_generator_flat.h"

#include "voxel/chunk.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    ChunkGenerator_Flat::ChunkGenerator_Flat(const ChunkSystem& chunk_system)
        : ChunkGenerator_Base(chunk_system)
    {}

    void ChunkGenerator_Flat::gen_chunk(Chunk& chunk)
    {
        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 y = 0; y < voxel_constants::chunk_size; y++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            i32 world_y = chunk.chunk_pos().y * voxel_constants::chunk_size + y;
            if (world_y < block_layers.size())
            {
                chunk.set_block_at({ x, y, z }, block_layers[world_y]);
            }
            else
            {
                chunk.set_block_at({ x, y, z }, Block::Air);
            }
        }
    }
}