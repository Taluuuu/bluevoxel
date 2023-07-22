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
        if (player_chunk_pos == m_last_player_chunk_pos)
            return;

        on_player_changed_chunk.broadcast({ player_chunk_pos });
        m_last_player_chunk_pos = player_chunk_pos;

//        const std::vector<ChunkPtr> old_loaded_chunks = std::move(m_loaded_chunks);
//        m_loaded_chunks = std::vector<ChunkPtr>();
//
//        for (i32 i = 0; i < m_world_size; i++)
//        for (i32 j = 0; j < m_world_size; j++)
//        {
//            const i32 offset = m_world_size / 2;
//            const v2i world_pos = {
//                i + player_chunk_pos.x - offset,
//                j + player_chunk_pos.z - offset };
//
//            const auto chunk_col_it = m_world_chunks.find(world_pos);
//            if (chunk_col_it != m_world_chunks.end())
//            {
//                // Copy found chunk column to loaded chunks.
//                m_loaded_chunks.insert(
//                    m_loaded_chunks.end(),
//                    chunk_col_it->second.begin(),
//                    chunk_col_it->second.end());
//
//                continue;
//            }
//
//            // No chunk column was found at this location; create new.
//
//        }
    }

    ChunkColumnPtr ChunkSystem::fetch_chunk_column(v2i chunk_location) const
    {
        const auto it = m_loaded_chunks.find(chunk_location);
        return (it == m_loaded_chunks.end()) ? nullptr : it->second;
    }

    ChunkColumnPtr ChunkSystem::fetch_or_create_chunk_column(v2i chunk_location)
    {
        if (auto chunk_col = fetch_chunk_column(chunk_location))
            return chunk_col;

        auto chunk_col = create_chunk_column(chunk_location);
        m_loaded_chunks[chunk_location] = chunk_col;
        return chunk_col;
    }

    ChunkColumnPtr ChunkSystem::create_chunk_column(v2i chunk_location) const
    {
        auto chunk_col = std::make_shared<ChunkColumn>();

        i32 y = 0;
        for (auto& chunk : *chunk_col)
        {
            chunk.init(*this, { chunk_location.x, y++, chunk_location.y });
            m_chunk_generator->gen_chunk(chunk);
            on_chunk_updated.broadcast({ chunk });
        }

        return chunk_col;
    }
}