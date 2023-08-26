#include "voxel/chunk_generators/chunk_generator_flat.h"

#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    ChunkGenerator_Flat::ChunkGenerator_Flat(const ChunkSystem& chunk_system)
        : ChunkGenerator_Base(chunk_system)
    {}

    void ChunkGenerator_Flat::run_generation_step(ChunkColumn& chunk_col, StaticChunkRegion& chunk_region) const
    {
        assert(chunk_col.generation_stage() == 0);

        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            i32 y = 0;
            for (const Block& layer : block_layers)
            {
                const i32 chunk_y = y / voxel_constants::chunk_size;

                chunk_col[chunk_y].set_block_at({ x, y, z }, layer);

                y++;
            }
        }

        // Really bad, for testing only
        // chunk_col[0].set_block_at({ 0, 31, 0 }, { 4, 0 });

        chunk_col.finish_generation();
    }
}