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
        while (!chunk_load_queue.empty() && left_to_load > 0)
        {
            const auto& request = chunk_load_queue[0];
            const auto chunk_col = create_chunk_column(request.chunk_pos);

            m_loaded_chunks[request.chunk_pos] = chunk_col;
            request.fetch_callback(chunk_col);

            chunk_load_queue.erase(chunk_load_queue.cbegin());
            left_to_load--;
        }
    }

    ChunkColumnPtr ChunkSystem::fetch_chunk_column(v2i chunk_location) const
    {
        const auto it = m_loaded_chunks.find(chunk_location);
        return (it == m_loaded_chunks.end()) ? nullptr : it->second;
    }

    void ChunkSystem::fetch_or_create_chunk_column(
        v2i chunk_location,
        const std::function<void(const ChunkColumnPtr&)>& chunk_fetch_callback)
    {
        if (auto chunk_col = fetch_chunk_column(chunk_location))
        {
            chunk_fetch_callback(chunk_col);
            return;
        }

        chunk_load_queue.push_back({ chunk_location, chunk_fetch_callback });
    }

    ChunkColumnPtr ChunkSystem::create_chunk_column(v2i chunk_location) const
    {
        auto chunk_col = std::make_shared<ChunkColumn>();

        i32 y = 0;
        for (auto& chunk : *chunk_col)
        {
            chunk.init(*this, { chunk_location.x, y++, chunk_location.y });
            m_chunk_generator->gen_chunk(chunk);
        }

        return chunk_col;
    }
}