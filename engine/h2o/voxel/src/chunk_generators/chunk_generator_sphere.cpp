#include "voxel/chunk_generators/chunk_generator_sphere.h"

#include "voxel/chunk_column.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    ChunkGenerator_Sphere::ChunkGenerator_Sphere(const ChunkSystem& chunk_system)
        : ChunkGenerator_Base(chunk_system)
    {}

    void ChunkGenerator_Sphere::run_generation_step(ChunkColumn& chunk_col, StaticChunkRegion& chunk_region) const
    {
        if (chunk_col.generation_stage() == 0)
        {
            const i32 sphere_radius = voxel_constants::vertical_block_count / 4;
            const v3i sphere_center { 0, sphere_radius, 0 };

            for (auto& chunk : chunk_col)
            {
                for (i32 x = 0; x < voxel_constants::chunk_size; x++)
                for (i32 y = 0; y < voxel_constants::chunk_size; y++)
                for (i32 z = 0; z < voxel_constants::chunk_size; z++)
                {
                    const v3i world_pos = v3i{ x, y, z } + chunk.chunk_pos() * voxel_constants::chunk_size;

                    // Long thing
//                    const f32 y_lim = 10000.0f / (world_pos.x * world_pos.x + world_pos.z * world_pos.z);
//                    if (world_pos.y < y_lim)
//                        chunk.set_block_at({x, y, z}, 1);

                    // Sphere
                    const float distance = glm::distance(v3(world_pos), v3(sphere_center));
                    if (distance < sphere_radius)
                    {
                        if (distance > sphere_radius / 2)
                            chunk.set_block_at({x, y, z}, 1);
                    }
                }
            }

            chunk_col.finish_generation();
        }
    }
}