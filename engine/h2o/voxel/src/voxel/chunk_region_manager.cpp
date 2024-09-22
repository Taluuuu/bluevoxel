#include "voxel/chunk_region_manager.h"

#include "core/engine.h"
#include "voxel/chunk_manager.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_utils.h"

#include <mutex>
#include <voxel/chunk_generators/chunk_generator_base.h>
#include <voxel_server/chunk_server.h>

namespace h2o
{
    ChunkRegionManager::ChunkRegionManager(ChunkServer& chunk_server)
        : m_chunk_server(&chunk_server)
    {}

    void ChunkRegionManager::generate_regions_for_chunk(v2i chunk_pos)
    {
        std::vector<v2i> regions_to_generate{};

        // Find regions that need generating
        {
            const std::shared_lock lock{ m_chunk_regions_mutex };

            const v2i center_region_pos = voxel_utils::chunk_to_region_pos(chunk_pos);
            for (i32 i = -1; i <= 1; i++)
            for (i32 j = -1; j <= 1; j++)
            {
                const v2i offset{ i, j };
                const v2i offset_region_pos = center_region_pos + offset;

                if (!m_chunk_regions.contains(offset_region_pos))
                    regions_to_generate.push_back(offset_region_pos);
            }
        }

        if (!regions_to_generate.empty())
        {
            // Only exclusively lock the mutex if there are regions that need to be generated
            const std::unique_lock lock{ m_chunk_regions_mutex };

            for (v2i region_pos : regions_to_generate)
            {
                if (auto& region = m_chunk_regions[region_pos]; !region)
                {
                    region = std::make_shared<ChunkRegionData>(region_pos);

                    g_engine->thread_pool().queue_job(0.0f,
                        [this, region_pos]
                        {
                            generate_region_terrain(region_pos);
                            update_chunk_generation_states(region_pos);
                        }
                    );
                }
            }
        }
    }

    void ChunkRegionManager::fetch_region(v2i region_pos, const std::function<void(const ChunkRegion*)>& function) const
    {
        fetch_region_data(region_pos,
            [&](const ChunkRegionData* region_data)
            {
                if (region_data)
                {
                    std::shared_lock lock{ region_data->mutex };
                    function(&region_data->chunk_region);
                    return;
                }

                function(nullptr);
            }
        );
    }

    void ChunkRegionManager::fetch_region_mut(v2i region_pos, const std::function<void(ChunkRegion*)>& function)
    {
        fetch_region_data_mut(region_pos,
            [&](ChunkRegionData* region_data)
            {
                if (region_data)
                {
                    std::unique_lock lock{ region_data->mutex };
                    function(&region_data->chunk_region);
                    return;
                }

                function(nullptr);
            }
        );
    }

    void ChunkRegionManager::fetch_region_data(v2i region_pos, const std::function<void(const ChunkRegionData*)>& function) const
    {
        const ChunkRegionData* region_data = nullptr;
        {
            const std::shared_lock regions_lock{ m_chunk_regions_mutex };
            if (const auto it = m_chunk_regions.find(region_pos); it != m_chunk_regions.end())
                region_data = it->second.get();
        }

        function(region_data);
    }

    void ChunkRegionManager::fetch_region_data_mut(v2i region_pos, const std::function<void(ChunkRegionData*)>& function)
    {
        ChunkRegionData* region_data = nullptr;
        {
            const std::shared_lock regions_lock{ m_chunk_regions_mutex };
            if (const auto it = m_chunk_regions.find(region_pos); it != m_chunk_regions.end())
                region_data = it->second.get();
        }

        function(region_data);
    }

    void ChunkRegionManager::generate_region_terrain(v2i region_pos)
    {
        fetch_region_mut(region_pos,
            [&](ChunkRegion* region)
            {
                assert(region);
                assert(region->generation_state == ChunkRegion::GenerationState::Pending);

                const v2i corner = voxel_utils::region_to_chunk_pos(region_pos);
                m_chunk_server->chunk_mgr().view_or_create_mut<ChunkRegionExtents>(
                    { corner.x, 0, corner.y },
                    [&](ChunkRegionView& region_view)
                    {
                        if (const auto chunk_generator = m_chunk_server->chunk_generator())
                        {
                            chunk_generator->gen_blocks(region_view);
                            region->register_structures(chunk_generator->gen_structures(region_view));
                        }
                    }
                );

                region->generation_state = ChunkRegion::GenerationState::Terrain;
            }
        );
    }

    void ChunkRegionManager::update_chunk_generation_states(v2i region_pos)
    {
        // Returns structures that should be placed in the region at region_pos, or nullopt if
        // not all regions neighbouring this one are generated.
        const auto find_structures_to_place =
            [this](v2i region_pos) -> std::optional< std::vector<VoxelStructureInstance> >
            {
                std::vector<VoxelStructureInstance> result{};

                const std::shared_lock regions_lock{ m_chunk_regions_mutex };
                for (i32 i = -1; i <= 1; i++)
                for (i32 j = -1; j <= 1; j++)
                {
                    const v2i offset_region_pos = region_pos + v2i{ i, j };

                    const auto it = m_chunk_regions.find(offset_region_pos);

                    // Returning nullopt here as would be logical causes some holes in the world that are
                    // never generated. Doing continue here might lead to some structures spawning in an
                    // incomplete way. Need to investigate this further.
                    if (it == m_chunk_regions.end())
                        // return std::nullopt;
                        continue;

                    if (!it->second)
                        return std::nullopt; // OK

                    auto& [region, mutex] = *it->second;
                    const std::shared_lock region_lock{ mutex };
                    if (region.generation_state < ChunkRegion::GenerationState::Terrain)
                        return std::nullopt;

                    const auto& region_structures = region.structures();
                    result.insert(result.end(), region_structures.begin(), region_structures.end());
                }

                return result;
            };

        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            const v2i offset_region_pos = region_pos + v2i{ i, j };

            const auto structures = find_structures_to_place(offset_region_pos);
            if (!structures)
                continue;

            const v2i offset_region_corner = voxel_utils::region_to_chunk_pos(region_pos);
            m_chunk_server->chunk_mgr().view_mut<ChunkRegionExtents>(
                { offset_region_corner.x, 0, offset_region_corner.y },
                [&](ChunkRegionView& region_view)
                {
                    region_view.for_each_chunk(
                        [&](Chunk& chunk)
                        {
                            for (const auto& structure : *structures)
                                chunk.place_structure(structure);

                            chunk.mark_generated();
                        }
                    );
                }
            );

            fetch_region_mut(offset_region_pos,
                [](ChunkRegion* chunk_region)
                {
                    if (chunk_region)
                        chunk_region->generation_state = ChunkRegion::GenerationState::Structures;
                }
            );
        }
    }
}
