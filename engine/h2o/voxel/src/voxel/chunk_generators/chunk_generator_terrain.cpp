#include "voxel/chunk_generators/chunk_generator_terrain.h"

#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"

#include <FastNoise/FastNoise.h>

namespace h2o
{
    ChunkGenerator_Terrain::ChunkGenerator_Terrain()
        : m_noise_generator(FastNoise::NewFromEncodedNodeTree("GQAbABkAEwCmm8Q7DQAFAAAAAAAAQAcAAFyPwj4AAAAAAAAAAIA/AAAAcEEAAADwQg=="))
    {}

    void ChunkGenerator_Terrain::gen_blocks(ChunkRegionView& region_view) const
    {
        // In chunk coordinates
        const v2i region_corner{ region_view.corner_chunk_pos().x, region_view.corner_chunk_pos().z };
        const v2i region_size{ region_view.size().x, region_view.size().z };

        // In block coordinates
        const v2i region_corner_blocks = region_corner * v2i{ voxel_constants::chunk_size };
        const v2i region_size_blocks = region_size * v2i{ voxel_constants::chunk_size };

        std::vector<f32> noise_outputs(region_size_blocks.x * region_size_blocks.y, 0.0f);
        m_noise_generator->GenUniformGrid2D(
            noise_outputs.data(),
            region_corner_blocks.x, region_corner_blocks.y,
            region_size_blocks.x, region_size_blocks.y, 1.0f, 69);

        for (i32 x = 0; x < region_size_blocks.x; x++)
        for (i32 z = 0; z < region_size_blocks.y; z++)
        {
            const i32 ground_level = i32(noise_outputs[z * region_size_blocks.x + x]) + 64;
            for (i32 y = 0; y < ground_level; y++)
            {
                const Block block = 3; // stone
                region_view.set_block_at({ x, y, z }, block, ViewRelativeTo::ViewCorner);
            }
        }
    }

    std::vector<VoxelStructureInstance> ChunkGenerator_Terrain::gen_structures(const ChunkRegionView& region_view) const
    {
        return {};
    }
}