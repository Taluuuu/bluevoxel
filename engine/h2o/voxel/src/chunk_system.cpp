#include "voxel/chunk_system.h"

#include "scene/scene.h"
#include "voxel/chunk.h"
#include "voxel/chunk_generator_flat.h"
#include "voxel/voxel_constants.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkSystem::ChunkSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        // Setup flat world gen
        auto flat_generator = std::make_unique<ChunkGenerator_Flat>(*this);
        flat_generator->block_layers = { 3, 3, 3, 3, 3, 2, 2, 2, 1 };
        m_chunk_generator = std::move(flat_generator);

        set_tick_phases(Update);
    }

    bool ChunkSystem::init()
    {
        return true;
    }

    void ChunkSystem::update(f32 delta_time)
    {
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

        // Load new chunk(s)
        i32 left_to_load = 1;
        while (!m_chunk_load_queue.empty() && left_to_load > 0)
        {
            const auto request = util::distance_queue_pop(m_chunk_load_queue);
            assert(request);

            // Check if chunk already exists
            WeakHandle<ChunkColumn> weak_chunk_col { nullptr };
            if (const auto chunk_col = fetch_chunk_column(request->chunk_pos))
                weak_chunk_col = chunk_col;

            if (!weak_chunk_col)
            {
                auto chunk_col = create_chunk_column(request->chunk_pos);
                weak_chunk_col = chunk_col;

                m_loaded_chunks[request->chunk_pos] = std::move(chunk_col);
            }

            assert(weak_chunk_col);
            request->fetch_callback(weak_chunk_col);

            left_to_load--;
        }
    }

    WeakHandle<ChunkColumn> ChunkSystem::fetch_chunk_column(v2i chunk_location) const
    {
        const auto it = m_loaded_chunks.find(chunk_location);
        return (it == m_loaded_chunks.end()) ? nullptr : WeakHandle<ChunkColumn>(it->second);
    }

    void ChunkSystem::fetch_or_create_chunk_column(
        v2i chunk_location, f32 distance, const ChunkFetchCallback& chunk_fetch_callback)
    {
        if (auto chunk_col = fetch_chunk_column(chunk_location))
        {
            chunk_fetch_callback(chunk_col);
            return;
        }

        // In the future we might want to add every request, but right now
        // it's not needed and slows things down.
        if (!util::distance_queue_contains_by_predicate<ChunkLoadRequest>(m_chunk_load_queue,
            [chunk_location](const ChunkLoadRequest& request) -> bool
            {
                return request.chunk_pos == chunk_location;
            }))
        {
            util::distance_queue_insert(
                m_chunk_load_queue,
                ChunkLoadRequest { chunk_location, chunk_fetch_callback }, distance);
        }
    }

    OwningHandle<ChunkColumn> ChunkSystem::create_chunk_column(v2i chunk_location) const
    {
        auto chunk_col = oup::make_observable_unique<ChunkColumn>();

        i32 y = 0;
        for (auto& chunk : *chunk_col)
        {
            chunk.init({ chunk_location.x, y++, chunk_location.y });
            m_chunk_generator->gen_chunk(chunk);
        }

        return chunk_col;
    }
}