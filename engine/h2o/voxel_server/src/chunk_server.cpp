#include "voxel_server/chunk_server.h"

#include "core/engine.h"
#include "core/log.h"
#include "voxel/chunk_generators/chunk_generator_base.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    ChunkServer::~ChunkServer()
    {
        stop();
    }

    void ChunkServer::start()
    {
        if (!m_thread.joinable())
        {
            m_should_stop = false;
            m_thread = std::thread(&ChunkServer::run, this);
        }
    }

    void ChunkServer::stop()
    {
        if (m_thread.joinable())
        {
            m_should_stop = true;
            m_thread.join();
        }
    }

    void ChunkServer::register_client(ClientID client_id)
    {
        std::lock_guard lock(m_clients_mutex);

        const auto it = m_clients.find(client_id);
        if (it == m_clients.end())
        {
            m_clients[client_id] = std::make_unique<VoxelClient>();
        }
        else
        {
            log::warn("Trying to register two chunk loaders with the same id: {}", client_id);
        }
    }

    void ChunkServer::unregister_client(ClientID client_id)
    {
        std::lock_guard lock(m_clients_mutex);

        if (m_clients.erase(client_id) == 0)
        {
            log::warn("Trying to remove a chunk loader with id '{}' when none was found.", client_id);
        }
    }

    void ChunkServer::set_chunk_generator(std::unique_ptr<ChunkGenerator_Base>&& chunk_generator)
    {
        m_chunk_generator = std::move(chunk_generator);
    }

    void ChunkServer::run()
    {
        std::deque<ChunkGenRequest> chunk_gen_deque{};

        while (!m_should_stop)
        {
            request_chunk_loads(chunk_gen_deque);
            load_requested_chunks(chunk_gen_deque);
        }
    }

    static void process_load_request(
        ClientID client_id,
        v2i chunk_pos,
        v2i client_pos,
        i32 target_generation_level,
        std::priority_queue<ChunkGenRequest>& gen_request_queue,
        std::queue< std::shared_ptr<ChunkColumn> >& loaded_chunks,
        ChunkManager& chunk_mgr)
    {
        auto chunk_col = chunk_mgr.fetch_chunk_at(chunk_pos);
        if (chunk_col && chunk_col->is_generated())
        {
            loaded_chunks.push(chunk_col);
            return;
        }

        const f32 distance = glm::length(v2(chunk_pos - client_pos));
        const ChunkRegion chunk_region { chunk_pos, chunk_mgr };

        gen_request_queue.emplace(
            chunk_region, target_generation_level, distance, client_id);
    }

    void ChunkServer::request_chunk_loads(std::deque<ChunkGenRequest>& chunk_gen_deque)
    {
        assert(m_chunk_generator);

        const i32 gen_level = m_chunk_generator->max_generation_stage();

        std::priority_queue<ChunkGenRequest> gen_request_queue{};

        {
            std::lock_guard clients_lock { m_clients_mutex };

            for (auto& [client_id, client]: m_clients)
            {
                assert(client);

                std::lock_guard client_lock{client->mutex};

                auto& input = client->input;
                auto& output = client->output;
                auto& load_requests = input.load_requests;

                while (!load_requests.empty())
                {
                    process_load_request(
                        client_id,
                        load_requests.front(),
                        input.position,
                        gen_level,
                        gen_request_queue,
                        output.loaded_chunks,
                        m_chunk_mgr);

                    load_requests.pop();
                }
            }
        }

        while (!gen_request_queue.empty())
            chunk_gen_deque.push_back(gen_request_queue.top());
    }

    void ChunkServer::load_requested_chunks(std::deque<ChunkGenRequest>& chunk_gen_deque)
    {
        auto voxel_module = g_engine->get_module<VoxelModule>();
        assert(voxel_module);

        while (!chunk_gen_deque.empty())
        {
            auto& gen_request = chunk_gen_deque.front();

            // Chunk columns that need to be generated up to gen_request.generation_stage - 1
            std::vector< std::shared_ptr<ChunkColumn> > chunk_cols_to_generate;
            chunk_cols_to_generate.reserve(9);

            gen_request.gen_region.for_each_chunk_column(
                [&](const std::shared_ptr<ChunkColumn>& chunk_col) -> void
                {
                    assert(chunk_col);
                    if (chunk_col->generation_stage() < gen_request.gen_stage - 1)
                        chunk_cols_to_generate.push_back(chunk_col);
                }
            );

            if (chunk_cols_to_generate.empty())
            {
                auto& chunk_col = gen_request.gen_region.center_chunk();

                // Init the chunk just before starting generation
                if (!chunk_col->is_initialized())
                    chunk_col->init(*voxel_module);

                // Gen request can be completed
                m_chunk_generator->run_generation_step(gen_request.gen_region);

                if (chunk_col->is_generated())
                {
                    std::lock_guard clients_lock { m_clients_mutex };

                    auto client_it = m_clients.find(gen_request.requester_id);
                    if (client_it != m_clients.end())
                    {
                        auto& client = client_it->second;

                        std::lock_guard client_lock { client->mutex };

                        client->output.loaded_chunks.push(chunk_col);
                    }
                }

                chunk_gen_deque.pop_front();
            }
            else
            {
                for (const auto& chunk_col : chunk_cols_to_generate)
                {
                    const ChunkRegion gen_region { chunk_col->chunk_column_pos(), m_chunk_mgr };
                    
                    chunk_gen_deque.emplace_front(
                        gen_region,
                        gen_request.gen_stage - 1,
                        gen_request.distance,
                        gen_request.requester_id);
                }
            }
        }
    }
}