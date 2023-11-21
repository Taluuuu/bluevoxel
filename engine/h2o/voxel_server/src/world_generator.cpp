#include "voxel_server/world_generator.h"

#include "core/engine.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_manager_base.h"

namespace h2o
{
    WorldGenerator::WorldGenerator(ChunkManager_Base& chunk_mgr)
        : m_chunk_mgr(&chunk_mgr)
    {

    }

    void WorldGenerator::request_chunk_column(
        v2i chunk_column_pos,
        const ChunkRequestFinishedCallback& on_completed)
    {
        assert(m_chunk_mgr);
        assert(m_chunk_generator);

        bool chunk_is_generated = false;
        m_chunk_mgr->fetch_chunk_column(chunk_column_pos, false,
            [&](const ChunkColumn* chunk_column)
            {
                if (chunk_column && chunk_column->is_generated())
                {
                    on_completed(*chunk_column);
                    chunk_is_generated = true;
                }
            }
        );

        if (chunk_is_generated)
            return;

        std::lock_guard lock { m_mutex };

        const auto it = m_chunks_pending_generation.find(chunk_column_pos);
        if (it != m_chunks_pending_generation.end())
        {
            // Just add the listener
            it->second.push_back(on_completed);
            return;
        }

        // Queue generating the chunk
        m_chunks_pending_generation[chunk_column_pos].push_back(on_completed);
        g_engine->thread_pool().queue_job(0.0f,
            [&, chunk_column_pos]
            {
                m_chunk_mgr->fetch_or_create_chunk_column(chunk_column_pos, true,
                    [&](ChunkColumn& chunk_column, bool _)
                    {
                        generate_chunk(chunk_column);
                    }
                );
            }
        );
    }

    void WorldGenerator::set_chunk_generator(std::unique_ptr<ChunkGenerator_Base>&& chunk_generator)
    {
        m_chunk_generator = std::move(chunk_generator);
    }

    void WorldGenerator::generate_chunk(ChunkColumn& chunk_column)
    {
        while (!chunk_column.is_generated())
            m_chunk_generator->run_generation_step(chunk_column);

        const v2i chunk_column_pos = chunk_column.chunk_column_pos();

        std::lock_guard lock { m_mutex };
        for (const auto& callback : m_chunks_pending_generation[chunk_column_pos])
            callback(chunk_column);

        m_chunks_pending_generation.erase(chunk_column_pos);
    }
}