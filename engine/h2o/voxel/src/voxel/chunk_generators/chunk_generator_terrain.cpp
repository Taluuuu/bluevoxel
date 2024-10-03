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
                const Block block = 1; // stone
                region_view.set_block_at({ x, y, z }, block, ViewRelativeTo::ViewCorner);
            }
        }
    }

    std::vector<VoxelStructureInstance> ChunkGenerator_Terrain::gen_structures(const ChunkRegionView& region_view) const
    {
        std::vector<VoxelStructureInstance> structures{};

        const v3i corner = region_view.corner_chunk_pos() * voxel_constants::chunk_size;

        srand(corner.x ^ corner.z);
        for (i32 i = 0; i < voxel_constants::chunk_region_block_count; i++)
        for (i32 j = 0; j < voxel_constants::chunk_region_block_count; j++)
        {
            const f32 random_float = f32(rand()) / std::numeric_limits<i32>::max();
            if (random_float > 0.00005f)
                continue;

            // Find ground level. This should probably be an easily accessible function
            i32 ground_level = 0;
            for (; ground_level < voxel_constants::vertical_block_count; ground_level++)
            {
                const auto block = region_view.get_block_at({ i, ground_level, j }, ViewRelativeTo::ViewCorner);
                if (!block || block == Block::Air)
                    break;
            }

            const v3i structure_pos = corner + v3i{ i, ground_level, j };
            structures.emplace_back(2, structure_pos);
        }

        return structures;
    }
}