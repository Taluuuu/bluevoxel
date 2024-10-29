#include "voxel/chunk_generators/chunk_generator_terrain.h"

#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"

#include <FastNoise/FastNoise.h>
#include <fstream>

namespace h2o
{
    static std::vector<f32> gen_noise(const v3i& corner, const v3i& size, const i32 num_octaves, const f32 frequency)
    {
        const FastNoise::SmartNode<> simplex = FastNoise::New<FastNoise::OpenSimplex2S>();

        std::vector<f32> result(size.x * size.y * size.z, 0.0f);
        for (i32 i = 0; i < num_octaves; i++)
        {
            std::vector<f32> noise_outputs(size.x * size.y * size.z, 0.0f);
            simplex->GenUniformGrid3D(noise_outputs.data(),
                corner.x, corner.y, corner.z,
                size.x, size.y, size.z,
                frequency * f32(i + 1), 0);

            for (i32 j = 0; j < noise_outputs.size(); j++)
                result[j] += noise_outputs[j] / f32(i + 1);
        }

        return result;
    }

    ChunkGenerator_Terrain::ChunkGenerator_Terrain()
        : m_graph(v2{0.0f}, v2{ voxel_constants::vertical_block_count, 1.0f })
    {
    }

    void ChunkGenerator_Terrain::gen_blocks(ChunkRegionView& region_view) const
    {
        // In chunk coordinates
        const v3i region_corner{ region_view.corner_chunk_pos().x, 0, region_view.corner_chunk_pos().z };
        const v3i region_size = region_view.size();

        // In block coordinates
        const v3i region_corner_blocks = region_corner * v3i{ voxel_constants::chunk_size };
        const v3i region_size_blocks = region_size * v3i{ voxel_constants::chunk_size };

        const std::vector<f32> noise_outputs = gen_noise(region_corner_blocks, region_size_blocks, 1, 0.01f);

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

            if (noise_val > air_threshold)
            {
                const Block block = 1;
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
