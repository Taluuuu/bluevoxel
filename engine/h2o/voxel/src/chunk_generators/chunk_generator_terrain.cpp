#include "voxel/chunk_generators/chunk_generator_terrain.h"

#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"

#include <FastNoise/FastNoise.h>

namespace h2o
{
    ChunkGenerator_Terrain::ChunkGenerator_Terrain()
        : m_noise_generator(FastNoise::NewFromEncodedNodeTree("GQAbABkAEwCmm8Q7DQAFAAAAAAAAQAcAAFyPwj4AAAAAAAAAAIA/AAAAcEEAAADwQg=="))
    {

    }

    // TODO: Could the ChunkRegion be an interface instead ?
    void ChunkGenerator_Terrain::run_generation_step(ChunkColumn& chunk_column) const
    {
        assert(m_noise_generator);
        assert(!block_layers.empty());
        assert(chunk_column.generation_stage() == 0);

        const v2i chunk_column_pos = chunk_column.chunk_column_pos();
        std::vector<f32> noise_outputs(voxel_constants::chunk_area, 0.0f);
        m_noise_generator->GenUniformGrid2D(
            noise_outputs.data(),
            chunk_column_pos.x * voxel_constants::chunk_size,
            chunk_column_pos.y * voxel_constants::chunk_size,
            voxel_constants::chunk_size,
            voxel_constants::chunk_size,
            1.0f,
            69);

        for (i32 x = 0; x < voxel_constants::chunk_size; x++)
        for (i32 z = 0; z < voxel_constants::chunk_size; z++)
        {
            const i32 ground_level = i32(noise_outputs[z * voxel_constants::chunk_size + x]);
            const i32 layers_start = ground_level - i32(block_layers.size());

            for (i32 y = 0; y < ground_level; y++)
            {
                Block block = block_layers[0]; // block_layers is not empty or an assert fails
                if (y >= layers_start)
                    block = block_layers[y - layers_start];

                const i32 chunk_y = y / voxel_constants::chunk_size;
                chunk_column[chunk_y].set_block_at({ x, y % voxel_constants::chunk_size, z }, block);
            }
        }

        chunk_column.finish_generation();
    }
}