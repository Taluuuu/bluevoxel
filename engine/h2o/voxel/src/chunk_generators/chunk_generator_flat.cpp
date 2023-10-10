#include "voxel/chunk_generators/chunk_generator_flat.h"

#include "voxel/chunk_column.h"
#include "voxel/chunk_manager.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    void ChunkGenerator_Flat::run_generation_step(ChunkRegion& chunk_region) const
    {
        auto& center_chunk = *chunk_region.center_chunk();

        assert(center_chunk.generation_stage() == 0);

        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            i32 y = 0;
            for (const Block& layer : block_layers)
            {
                const i32 chunk_y = y / voxel_constants::chunk_size;

                center_chunk[chunk_y].set_block_at({ x, y, z }, layer);

                y++;
            }
        }

        // Terrible, for testing only
        // chunk_col[0].set_block_at({ 0, 31, 0 }, { 4, 0 });

        center_chunk.finish_generation();
    }
}