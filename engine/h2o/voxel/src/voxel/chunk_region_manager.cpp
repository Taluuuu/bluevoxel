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
        const auto regions_to_generate = get_regions_to_generate_for_chunk(chunk_pos);
        if (regions_to_generate.empty())
            return;

        const std::unique_lock lock{ m_chunk_regions_mutex };
        for (v2i region_pos : regions_to_generate)
        {
            auto& region = m_chunk_regions[region_pos];
            region = std::make_shared<ChunkRegionData>(region_pos);
            region->chunk_region.generation_state = ChunkRegion::GenerationState::Pending;

            g_engine->thread_pool().queue_job(0.0f,
                [this, region_pos]
                {
                    generate_region(region_pos);
                    update_chunk_generation_states(region_pos);
                }
            );
        }
    }

    std::vector<v2i> ChunkRegionManager::pop_newly_generated_regions()
    {
        std::unique_lock lock{ m_newly_generated_regions_mutex };

        auto result = std::move(m_newly_generated_regions);
        m_newly_generated_regions.clear();

        return result;
    }

    void ChunkRegionManager::fetch_region(v2i region_pos, const std::function<void(const ChunkRegion*)>& function) const
    {
        fetch_region_data(region_pos,
            [&](const ChunkRegionData* region_data)
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

    void ChunkRegionManager::fetch_region(v2i region_pos, const std::function<void(ChunkRegion*)>& function)
    {
        fetch_region_data(region_pos,
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

    void ChunkRegionManager::fetch_region_data(v2i region_pos, const std::function<void(ChunkRegionData*)>& function)
    {
        ChunkRegionData* region_data = nullptr;
        {
            const std::shared_lock regions_lock{ m_chunk_regions_mutex };
            if (const auto it = m_chunk_regions.find(region_pos); it != m_chunk_regions.end())
                region_data = it->second.get();
        }

        function(region_data);
    }

    std::vector<v2i> ChunkRegionManager::get_regions_to_generate_for_chunk(v2i chunk_pos) const
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
                    if (!region || region->generation_state == ChunkRegion::GenerationState::None)
                        result.push_back(offset_region_pos);
                }
            );
        }

        return result;
    }

    bool ChunkRegionManager::is_region_fully_generated(v2i region_pos) const
    {
        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            v2i offset_pos = region_pos + v2i{ i, j };
            if (offset_pos == region_pos)
                continue;

            bool is_region_generated = false;
            fetch_region(offset_pos,
                [&](const ChunkRegion* chunk_region)
                {
                    is_region_generated =
                        chunk_region &&
                        chunk_region->generation_state == ChunkRegion::GenerationState::Generated;
                }
            );

            if (!is_region_generated)
                return false;
        }

        return true;
    }

    void ChunkRegionManager::update_chunk_generation_states(v2i region_pos) const
    {
        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            v2i offset_region_pos = region_pos + v2i{ i, j };
            if (!is_region_fully_generated(offset_region_pos))
                continue;

            const v2i offset_region_corner = voxel_utils::region_to_chunk_pos(offset_region_pos);
            m_chunk_server->chunk_mgr().view_mut<ChunkRegionExtents>(
                { offset_region_corner.x, 0, offset_region_corner.y },
                [](ChunkRegionView& region_view)
                {
                    region_view.for_each_chunk(
                        [](Chunk& chunk) { chunk.mark_generated(); });
                }
            );
        }
    }

    void ChunkRegionManager::generate_region(v2i region_pos)
    {
        fetch_region(region_pos,
            [&](ChunkRegion* region)
            {
                if (region->generation_state == ChunkRegion::GenerationState::Generated)
                    return;

                const v2i corner = voxel_utils::region_to_chunk_pos(region_pos);
                m_chunk_server->chunk_mgr().view_or_create_mut<ChunkRegionExtents>(
                    { corner.x, 0, corner.y },
                    [&](ChunkRegionView& region_view)
                    {
                        const auto chunk_generator = m_chunk_server->chunk_generator();
                        if (!chunk_generator)
                            return;

                        chunk_generator->gen_blocks(region_view);

                        region->register_structures(chunk_generator->gen_structures(region_view));

                        region_view.for_each_chunk(
                            [&](Chunk& chunk) { region->place_structures(chunk); });
                    }
                );

                region->generation_state = ChunkRegion::GenerationState::Generated;

                std::unique_lock lock{ m_newly_generated_regions_mutex };
                m_newly_generated_regions.push_back(region_pos);
            }
        );
    }
}
