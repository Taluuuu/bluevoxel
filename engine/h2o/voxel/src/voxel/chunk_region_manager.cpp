#include "voxel/chunk_region_manager.h"

#include "core/engine.h"
#include "voxel/chunk_manager.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_utils.h"

#include <mutex>
#include <voxel/chunk_generators/chunk_generator_base.h>

namespace h2o
{
    ChunkRegionManager::ChunkRegionManager(
        ChunkManager& chunk_manager,
        const std::shared_ptr<ChunkGenerator_Base>& chunk_generator)
        : m_chunk_manager(&chunk_manager)
        , m_chunk_generator(chunk_generator)
    {}

    void ChunkRegionManager::generate_regions_for_chunk(v2i chunk_pos)
    {
        const auto regions_to_generate = get_regions_to_generate_before_chunk(chunk_pos);
        if (regions_to_generate.empty())
            return;

        const std::unique_lock lock{ m_chunk_regions_mutex };

        for (v2i region_pos : regions_to_generate)
        {
            auto& region = m_chunk_regions[region_pos];
            region = std::make_shared<ChunkRegionData>(region_pos);

            g_engine->thread_pool().queue_job(0.0f,
                [this, region_pos] { generate_region(region_pos); });
        }
    }

    void ChunkRegionManager::fetch_region(v2i region_pos, const std::function<void(const ChunkRegion* region)>& function) const
    {
        const std::shared_lock regions_lock{ m_chunk_regions_mutex };

        if (const auto it = m_chunk_regions.find(region_pos); it != m_chunk_regions.end())
        {
            if (const auto& region_data = it->second)
            {
                const auto& [region, mutex] = *region_data;

                const std::shared_lock region_lock{ mutex };
                function(&region);

                return;
            }
        }

        function(nullptr);
    }

    void ChunkRegionManager::fetch_region(v2i region_pos, const std::function<void(ChunkRegion* region)>& function)
    {
        const std::shared_lock regions_lock{ m_chunk_regions_mutex };

        if (const auto it = m_chunk_regions.find(region_pos); it != m_chunk_regions.end())
        {
            if (const auto& region_data = it->second)
            {
                auto& [region, mutex] = *region_data;

                const std::unique_lock region_lock{ mutex };
                function(&region);

                return;
            }
        }

        function(nullptr);
    }

    std::vector<v2i> ChunkRegionManager::get_regions_to_generate_before_chunk(v2i chunk_pos) const
    {
        std::vector<v2i> result{};

        const v2i region_pos = voxel_utils::chunk_to_region_pos(chunk_pos);
        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            const v2i offset{ i, j };
            const v2i offset_region_pos = region_pos + offset;

            fetch_region(offset_region_pos,
                [&](const ChunkRegion* region)
                {
                    if (!region || !region->is_generated())
                        result.push_back(offset_region_pos);
                }
            );
        }

        return result;
    }

    void ChunkRegionManager::generate_region(v2i region_pos)
    {
        fetch_region(region_pos,
            [&](ChunkRegion* region)
            {
                assert(!region->is_generated());

                const v2i corner = voxel_utils::region_to_chunk_pos(region_pos);
                m_chunk_manager->view_or_create<ChunkRegionExtents>(
                    { corner.x, 0, corner.y },
                    [&](ChunkRegionView& region_view)
                    {
                        m_chunk_generator->gen_blocks(region_view);

                        region->register_structures(m_chunk_generator->gen_structures(region_view));
                        region_view.for_each_chunk(
                            [&](Chunk& chunk)
                            { region->place_structures(chunk); }
                        );
                    }
                );
            }
        );

        on_finished_generating_region();
    }
}
