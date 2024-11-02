#include "voxel/chunk_generators/chunk_generator_terrain.h"

#include "voxel/chunk_region.h"
#include "voxel/chunk_view.h"
#include "voxel/voxel_constants.h"

#include <FastNoise/FastNoise.h>
#include <fstream>

namespace h2o
{
    static std::vector<f32> gen_noise(const v3i& corner, const v3i& size, const f32 frequency)
    {
        const auto generator = FastNoise::New<FastNoise::FractalFBm>();
        generator->SetSource(FastNoise::New<FastNoise::Simplex>());
        generator->SetOctaveCount(5);

        std::vector<f32> noise_outputs(size.x * size.y * size.z, 0.0f);
        generator->GenUniformGrid3D(noise_outputs.data(),
            corner.x, corner.y, corner.z,
            size.x, size.y, size.z,
            frequency, 0);

        return noise_outputs;
    }

    ChunkGenerator_Terrain::ChunkGenerator_Terrain()
        : m_graph(v2{0.0f}, v2{ voxel_constants::vertical_block_count, 1.0f })
    {
    }

    void ChunkGenerator_Terrain::gen_blocks(ChunkView& region_view) const
    {
        // In chunk coordinates
        const v3i region_corner{ region_view.corner_chunk_pos().x, 0, region_view.corner_chunk_pos().z };
        const v3i region_size = region_view.size();

        // In block coordinates
        const v3i region_corner_blocks = region_corner * v3i{ voxel_constants::chunk_size };
        const v3i region_size_blocks = region_size * v3i{ voxel_constants::chunk_size };

        const std::vector<f32> noise_outputs = gen_noise(region_corner_blocks, region_size_blocks, 0.002f);

        for (i32 z = 0; z < region_size_blocks.z; z++)
        for (i32 y = 0; y < region_size_blocks.y; y++)
        for (i32 x = 0; x < region_size_blocks.x; x++)
        {
            const size_t index =
                region_size_blocks.x * region_size_blocks.y * z +
                region_size_blocks.x * y +
                x;

            const f32 noise_val = (noise_outputs[index] + 1.0f) / 2.0f;
            const f32 air_threshold = m_graph.get_value_by_x(f32(y)).value_or(0.0f);

            if (noise_val < air_threshold)
            {
                const Block block = 3;
                region_view.set_block_at({ x, y, z }, block, ViewRelativeTo::ViewCorner);
            }
        }

        // Set dirt and grass
        for (i32 x = 0; x < region_size_blocks.x; x++)
        for (i32 z = 0; z < region_size_blocks.z; z++)
        {
            i32 num_blocks_under_air = 0;
            for (i32 y = region_size_blocks.y; y >= 0; --y)
            {
                const auto block = region_view.get_block_at({ x, y, z }, ViewRelativeTo::ViewCorner);
                if (block == Block::Air)
                {
                    num_blocks_under_air = 0;
                }
                else
                {
                    if (num_blocks_under_air == 0)
                    {
                        region_view.set_block_at({ x, y, z }, Block{ 1 }, ViewRelativeTo::ViewCorner);
                    }
                    else if (num_blocks_under_air < 5)
                    {
                        region_view.set_block_at({ x, y, z }, Block{ 2 }, ViewRelativeTo::ViewCorner);
                    }

                    num_blocks_under_air++;
                }
            }
        }
    }

    std::vector<VoxelStructureInstance> ChunkGenerator_Terrain::gen_structures(const ChunkView& region_view) const
    {
        std::vector<VoxelStructureInstance> structures{};

        const v3i corner = region_view.corner_chunk_pos() * voxel_constants::chunk_size;

        srand(corner.x ^ corner.z);
        for (i32 i = 0; i < voxel_constants::chunk_region_block_count; i++)
        for (i32 j = 0; j < voxel_constants::chunk_region_block_count; j++)
        {
            const f32 random_float = f32(rand()) / std::numeric_limits<i32>::max();

            const f32 tree_threshold = 0.01f;
            const f32 house_threshold = 0.00005f;

            if (random_float > tree_threshold)
                continue;

            // Find ground level. This should probably be an easily accessible function
            i32 ground_level = voxel_constants::vertical_block_count - 1;
            for (; ground_level >= 0; --ground_level)
            {
                const auto block = region_view.get_block_at({ i, ground_level, j }, ViewRelativeTo::ViewCorner);
                if (block != Block::Air)
                    break;
            }

            const v3i structure_pos = corner + v3i{ i, ground_level, j };
            if (random_float < house_threshold)
            {
                structures.emplace_back(2, structure_pos);
            }
            else if (random_float < tree_threshold)
            {
                structures.emplace_back(0, structure_pos);
            }
        }

        return structures;
    }

    void ChunkGenerator_Terrain::save(const fs::path& path) const
    {
        try
        {
            YAML::Emitter yaml{};

            yaml << YAML::BeginMap;

            yaml << YAML::Key << "graph";
            yaml << YAML::Value << m_graph;

            yaml << YAML::EndMap;

            std::ofstream file(path.string());
            file << yaml.c_str();

            log::info("Saved chunk generator to file at '{}'", absolute(path).string());
        }
        catch (const std::exception& e)
        {
            log::error("Failed to save chunk generator: {}", e.what());
        }
    }

    bool ChunkGenerator_Terrain::load(const fs::path& path)
    {
        try
        {
            const auto root_yml = YAML::LoadFile(path.string());

            m_graph = root_yml["graph"].as<Graph>();
            return true;
        }
        catch (const std::exception& e)
        {
            log::error("Failed to load chunk generator: {}", e.what());
            return false;
        }
    }
}
