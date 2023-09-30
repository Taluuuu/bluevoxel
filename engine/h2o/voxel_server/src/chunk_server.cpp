#include "voxel_server/chunk_server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/message_ids.h"
#include "networking/server.h"
#include "voxel/chunk_generators/chunk_generator_base.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_net_messages.h"

namespace h2o
{
    ChunkServer::ChunkServer(Server& server)
        : m_server(&server) {}

    ChunkServer::~ChunkServer()
    {
        stop();
    }

    void ChunkServer::start()
    {
        if (is_running())
            return;

        assert(m_server);

        // Bind messages
        m_server->handle_message<NetMsg_ChunkFetchRequest>(m_received_chunk_request_handle,
            [&](ClientID client_id, const NetMsg_ChunkFetchRequest& chunk_fetch_request)
            {
                log::info("Received {} chunk fetch requests.", chunk_fetch_request.requested_chunks.size());
                on_received_chunk_fetch_requests(client_id, chunk_fetch_request);
            }
        );

        m_should_stop = false;
        m_thread = std::thread(&ChunkServer::run, this);
    }

    void ChunkServer::stop()
    {
        if (!is_running())
            return;

        assert(m_server);

        // Unbind messages
        m_received_chunk_request_handle.reset();

        m_should_stop = true;
        m_thread.join();
    }

    bool ChunkServer::is_running() const
    {
        return m_thread.joinable();
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
            request_chunk_generations();
            load_requested_chunks();
        }
    }

    void ChunkServer::request_chunk_generations()
    {
        assert(m_chunk_generator);

        const i32 gen_level = m_chunk_generator->max_generation_stage();

        std::priority_queue<ChunkGenRequest> gen_request_queue{};

        {
            std::lock_guard fetch_requests_lock { m_chunk_fetch_requests_mutex };

            erase_if(m_chunk_fetch_requests,
                [&](const std::shared_ptr<ChunkFetchRequest>& chunk_fetch_request) -> bool
                {
                    assert(chunk_fetch_request);
                    auto& [clients, chunk_col_pos, is_generating] = *chunk_fetch_request;

                    if (is_generating)
                        return false;

                    if (auto chunk_col = m_chunk_mgr.fetch_chunk_at(chunk_col_pos))
                    {
                        if (chunk_col->is_generated())
                        {
                            send_chunk_column(*chunk_col, clients);
                            return true;
                        }
                    }

                    gen_request_queue.emplace(
                        chunk_fetch_request,
                        ChunkRegion(chunk_col_pos, m_chunk_mgr),
                        gen_level,
                        0.0f
                    );

                    is_generating = true;

                    return false;
                }
            );
        }

        std::lock_guard gen_dequeue_lock { m_chunk_gen_dequeue_mutex };
        while (!gen_request_queue.empty())
        {
            m_chunk_gen_deque.push_back(gen_request_queue.top());
            gen_request_queue.pop();
        }
    }

    void ChunkServer::load_requested_chunks()
    {
        auto voxel_module = g_engine->get_module<VoxelModule>();
        assert(voxel_module);

        std::lock_guard chunk_gen_deque_lock { m_chunk_gen_dequeue_mutex };

        while (!m_chunk_gen_deque.empty())
        {
            auto& [fetch_request, gen_region, gen_stage, distance] = m_chunk_gen_deque.front();

            // Chunk columns that need to be generated up to gen_request.generation_stage - 1
            std::vector< std::shared_ptr<ChunkColumn> > chunk_cols_to_generate;
            chunk_cols_to_generate.reserve(9);

            gen_region.for_each_chunk_column(
                [&](const std::shared_ptr<ChunkColumn>& chunk_col) -> void
                {
                    assert(chunk_col);
                    if (chunk_col->generation_stage() < gen_stage - 1)
                        chunk_cols_to_generate.push_back(chunk_col);
                }
            );

            if (chunk_cols_to_generate.empty())
            {
                auto& chunk_col = gen_region.center_chunk();

                // Init the chunk just before starting generation
                if (!chunk_col->is_initialized())
                    chunk_col->init(*voxel_module);

                // Gen request can be completed
                m_chunk_generator->run_generation_step(gen_region);

                if (chunk_col->is_generated() && fetch_request)
                    send_chunk_column(*chunk_col, { fetch_request->requesting_clients });

                m_chunk_gen_deque.pop_front();
            }
            else
            {
                for (const auto& chunk_col : chunk_cols_to_generate)
                {
                    m_chunk_gen_deque.emplace_front(
                        nullptr,
                        ChunkRegion { chunk_col->chunk_column_pos(), m_chunk_mgr },
                        gen_stage - 1,
                        0.0f);
                }
            }
        }
    }

    void ChunkServer::on_received_chunk_fetch_requests(
        ClientID client_id,
        const NetMsg_ChunkFetchRequest& chunk_fetch_request)
    {
        std::lock_guard lock { m_chunk_fetch_requests_mutex };

        for (v2i requested_chunk : chunk_fetch_request.requested_chunks)
        {
            // Either add this client id to an existing chunk fetch request or create a new one
            auto it = std::find_if(m_chunk_fetch_requests.begin(), m_chunk_fetch_requests.end(),
                [&](const auto& other) -> bool
                {
                    assert(other);
                    return other->chunk_col_pos == requested_chunk;
                }
            );

            if (it != m_chunk_fetch_requests.end())
            {
                auto& requesting_clients = (*it)->requesting_clients;

                if (std::find(
                    requesting_clients.begin(),
                    requesting_clients.end(), client_id) != requesting_clients.end())
                {
                    (*it)->requesting_clients.push_back(client_id);
                }
                continue;
            }

            const ChunkFetchRequest new_fetch_request{ { client_id }, requested_chunk };
            m_chunk_fetch_requests.push_back(std::make_shared<ChunkFetchRequest>(new_fetch_request));
        }
    }

    void ChunkServer::send_chunk_column(ChunkColumn& chunk_col, const std::vector<ClientID>& client_ids) const
    {
        // Send chunk column to requesting clients
        std::vector<CompressedChunk> compressed_chunks{};
        compressed_chunks.reserve(voxel_constants::vertical_chunk_count);

        for (const auto& chunk : chunk_col)
            compressed_chunks.push_back(chunk.compress());

        for (ClientID client: client_ids)
            m_server->send_message(client, NetMsg_ChunkFetchResult { compressed_chunks });
    }
}