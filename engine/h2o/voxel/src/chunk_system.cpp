#include "voxel/chunk_system.h"

#include "scene/scene.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_generators/chunk_generator_flat.h"
#include "voxel/chunk_generators/chunk_generator_sphere.h"
#include "voxel/voxel_constants.h"
#include "voxel/voxel_utils.h"
#include "voxel/chunk_region.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    ChunkSystem::ChunkSystem(
        const SceneSystemInitializer& system_initializer,
        const VoxelModule& voxel_module)
        : SceneSystem(system_initializer)
        , m_voxel_module(&voxel_module)
    {
        // Setup flat world gen
        auto flat_generator = std::make_unique<ChunkGenerator_Flat>(*this);
        flat_generator->block_layers = { 3, 3, 3, 3, 3, 2, 2, 2, 1 };
        m_chunk_generator = std::move(flat_generator);

//        m_chunk_gen_queues.resize(m_chunk_generator->max_generation_stage(), {});

        set_tick_phases(Update);
    }

    bool ChunkSystem::init()
    {
//        m_chunk_mgr.start();

        return true;
    }

    void ChunkSystem::update(f32 delta_time)
    {
        assert(m_voxel_module);

        // This is a bad way of getting the player, but it works for now.
        const auto player = m_scene->get_actor("Player");
        if (!player)
        {
            log::warn("Failed to get player in chunk system. You should find a better way to do this.");
            return;
        }

        const v3i player_chunk_pos = world_to_chunk_pos(player->transform.position);
        if (player_chunk_pos != m_last_player_chunk_pos)
        {
            on_player_changed_chunk.broadcast({m_last_player_chunk_pos, player_chunk_pos});
            m_last_player_chunk_pos = player_chunk_pos;
        }

        i32 chunk_generations = 0;
        while (!m_chunk_gen_stack.empty() && chunk_generations < 5)
        {
            auto& gen_request = m_chunk_gen_stack.front();

            // Chunk columns that need to be generated up to gen_request.generation_stage - 1
            std::vector<WeakHandle<ChunkColumn>> chunk_cols_to_generate;
            chunk_cols_to_generate.reserve(9);

            gen_request.gen_region.for_each_chunk_column(
                [&](const WeakHandle<ChunkColumn>& chunk_col) -> void
                {
                    assert(chunk_col);
                    if (chunk_col->generation_stage() < gen_request.gen_stage - 1)
                        chunk_cols_to_generate.push_back(chunk_col);
                }
            );

            if (chunk_cols_to_generate.empty())
            {
                auto& chunk_col = gen_request.chunk_column;

                // Init the chunk just before starting generation
                if (!chunk_col->is_initialized())
                    chunk_col->init(*m_voxel_module);

                // Gen request can be completed
                m_chunk_generator->run_generation_step(
                    *gen_request.chunk_column, gen_request.gen_region);

                if (gen_request.chunk_column->is_generated())
                {
                    on_chunk_column_loaded.broadcast({gen_request.chunk_column});

                    // Start ticking the chunk.
                    // TODO: Make an abstraction for this ? It seems kinda verbose
                    m_chunk_tick_queue.push(gen_request.chunk_column);
                }

                m_chunk_gen_stack.pop_front();
                chunk_generations++;
            }
            else
            {
                for (const auto& chunk_col : chunk_cols_to_generate)
                {
                    const StaticChunkRegion gen_region(
                        chunk_col->chunk_column_pos() - v2i{ 1, 1 }, 3, *this);

                    m_chunk_gen_stack.emplace_front(
                        gen_region, chunk_col, gen_request.gen_stage - 1);
                }
            }
        }

        const i32 chunks_to_tick = std::min(i32(m_chunk_tick_queue.size()), 5);
        for (i32 i = 0; i < chunks_to_tick; i++)
        {
            if (auto& chunk_col = m_chunk_tick_queue.front())
            {
                chunk_col->tick();
                m_chunk_tick_queue.push(chunk_col); // Add to the back of the queue again
            }

            m_chunk_tick_queue.pop();
        }
    }

    i32 ChunkSystem::num_chunks_waiting_generation() const
    {
        return i32(m_chunk_gen_stack.size());
    }

    WeakHandle<ChunkColumn> ChunkSystem::fetch_chunk_column(v2i chunk_col_pos) const
    {
        const auto it = m_loaded_chunks.find(chunk_col_pos);
        if (it == m_loaded_chunks.end())
            return nullptr;

        return { it->second };
    }

    WeakHandle<ChunkColumn> ChunkSystem::fetch_or_create_chunk_column(v2i chunk_col_pos)
    {
        if (const auto chunk_col = fetch_chunk_column(chunk_col_pos))
            return chunk_col; // Chunk column already exists

        // Need to create a new chunk column.
        // The chunk generation will come later on.
        auto chunk_col = create_chunk_column(chunk_col_pos);
        const WeakHandle<ChunkColumn> weak_chunk_col = chunk_col;
        assert(chunk_col);

        m_loaded_chunks[chunk_col_pos] = std::move(chunk_col);

        return weak_chunk_col;
    }

    void ChunkSystem::request_chunk_generation(const WeakHandle<ChunkColumn>& chunk_col, i32 queried_stage)
    {
        assert(m_chunk_generator);
        assert(chunk_col);

        if (queried_stage == voxel_constants::max_generation_stage)
            queried_stage = m_chunk_generator->max_generation_stage();

        assert(queried_stage <= m_chunk_generator->max_generation_stage());

        if (chunk_col->generation_stage() >= queried_stage)
            return;

        const StaticChunkRegion gen_region(
            chunk_col->chunk_column_pos() - v2i{ 1, 1 }, 3, *this);

        m_chunk_gen_stack.emplace_back(gen_region, chunk_col, queried_stage);
    }

    OwningHandle<ChunkColumn> ChunkSystem::create_chunk_column(v2i chunk_location) const
    {
        return oup::make_observable_unique<ChunkColumn>(chunk_location);
    }
}