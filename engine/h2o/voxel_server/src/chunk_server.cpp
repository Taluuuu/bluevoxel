#include "voxel_server/chunk_server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/message_ids.h"
#include "networking/server.h"
#include "voxel/chunk_generators/chunk_generator_base.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_utils.h"

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
        m_server->handle_message<net_msg::ChunkFetchRequest>(m_received_chunk_request_handle,
            [&](PeerID client_id, const net_msg::ChunkFetchRequest& chunk_fetch_request)
            {
                log::info("Received {} chunk fetch requests.", chunk_fetch_request.requested_chunks.size());
                on_received_chunk_fetch_requests(client_id, chunk_fetch_request);
            }
        );

        m_server->handle_message<net_msg::BlockPlaceRequest>(m_received_block_place_request_handle,
            [&](PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                log::info("Received block place request.");
                on_received_block_place_request(client_id, block_place_request);
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
        while (!m_should_stop)
        {
            request_chunk_generations();
            load_requested_chunks();
        }
    }

    void ChunkServer::request_chunk_generations()
    {
        assert(m_chunk_generator);

        const i32 target_gen_stage = m_chunk_generator->max_generation_stage();

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

                    bool chunk_was_sent = false;
                    m_chunk_mgr.fetch_chunk_column(chunk_col_pos, false,
                        [&](ChunkColumn* chunk_column)
                        {
                            if (chunk_column && chunk_column->is_generated())
                            {
                                send_chunk_column(*chunk_column, clients);
                                chunk_was_sent = true;
                            }
                        }
                    );

                    if (chunk_was_sent)
                        return true;

                    // Chunk either does not exist or is not fully generated yet.
                    gen_request_queue.emplace(
                        chunk_fetch_request,
                        target_gen_stage,
                        0.0f
                    );

                    is_generating = true;

                    return false;
                }
            );
        }

        // Push gen requests in reverse of distance
        std::lock_guard gen_dequeue_lock { m_chunk_gen_dequeue_mutex };
        while (!gen_request_queue.empty())
        {
            m_chunk_gen_deque.push_back(gen_request_queue.top());
            gen_request_queue.pop();
        }
    }

    void ChunkServer::load_requested_chunks()
    {
        std::lock_guard chunk_gen_deque_lock { m_chunk_gen_dequeue_mutex };

        while (!m_chunk_gen_deque.empty())
        {
            auto& [fetch_request, gen_stage, distance] = m_chunk_gen_deque.front();

            assert(fetch_request);

            // Chunk columns that need to be generated up to gen_request.generation_stage - 1
            std::vector<v2i> chunk_cols_to_generate;
            chunk_cols_to_generate.reserve(8);

            m_chunk_mgr.fetch_or_create_chunk_column(fetch_request->chunk_col_pos, true,
                [&](ChunkColumn& chunk_column, bool was_created)
                {
                    // Generate neighbour chunks that are not at least a generation stage behind this chunk column.
                    for (i32 i = -1; i <= 1; i++)
                    for (i32 j = -1; j <= 1; j++)
                    {
                        if (i == 0 && j == 0)
                            continue;

                        const v2i neighbour_chunk_col_pos = chunk_column.chunk_column_pos() + v2i{ i, j };
                        m_chunk_mgr.fetch_chunk_column(neighbour_chunk_col_pos, false,
                            [&](const ChunkColumn* neighbour_chunk_col)
                            {
                                if (!neighbour_chunk_col || neighbour_chunk_col->generation_stage() < gen_stage - 1)
                                    chunk_cols_to_generate.push_back(neighbour_chunk_col_pos);
                            }
                        );
                    }

                    if (chunk_cols_to_generate.empty())
                    {
                        // Gen request can be completed
                        m_chunk_generator->run_generation_step(chunk_column);

                        if (chunk_column.is_generated() && fetch_request)
                        {
                            send_chunk_column(chunk_column, { fetch_request->requesting_clients });

                            // TODO: Abstract this somewhat
                            auto chunk_fetch_request_it = std::find(
                                m_chunk_fetch_requests.begin(),
                                m_chunk_fetch_requests.end(), fetch_request);

                            if (chunk_fetch_request_it != m_chunk_fetch_requests.end())
                                m_chunk_fetch_requests.erase(chunk_fetch_request_it);
                        }

                        m_chunk_gen_deque.pop_front();
                    }
                    else
                    {
                        for (v2i chunk_col : chunk_cols_to_generate)
                        {
                            // TODO: This looks very weird.
                            const ChunkFetchRequest new_fetch_request{ {}, chunk_col, false };
                            auto new_fetch_request_ptr = std::make_shared<ChunkFetchRequest>(new_fetch_request);

                            m_chunk_fetch_requests.push_back(new_fetch_request_ptr);

                            m_chunk_gen_deque.emplace_front(
                                new_fetch_request_ptr,
                                gen_stage - 1,
                                0.0f);
                        }
                    }
                }
            );
        }
    }

    void ChunkServer::on_received_chunk_fetch_requests(
        PeerID client_id,
        const net_msg::ChunkFetchRequest& chunk_fetch_request)
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
                assert(*it);
                (*it)->requesting_clients.insert(client_id);
                continue;
            }

            const ChunkFetchRequest new_fetch_request{ { client_id }, requested_chunk };
            m_chunk_fetch_requests.push_back(std::make_shared<ChunkFetchRequest>(new_fetch_request));
        }
    }

    void ChunkServer::on_received_block_place_request(
        PeerID request_sender,
        const net_msg::BlockPlaceRequest& block_place_request)
    {
        if (!m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block))
            return;

        for (PeerID client_id : m_server->peers())
        {
            if (client_id == request_sender)
                continue;

            m_server->send_message(client_id, block_place_request);
        }
    }

    void ChunkServer::send_chunk_column(ChunkColumn& chunk_col, const std::set<PeerID>& client_ids) const
    {
        // Send chunk column to requesting clients
        std::vector<CompressedChunk> compressed_chunks{};
        compressed_chunks.reserve(voxel_constants::vertical_chunk_count);

        for (const auto& chunk : chunk_col)
            compressed_chunks.push_back(chunk.compress());

        for (PeerID client: client_ids)
        {
            m_server->send_message(client,
                net_msg::ChunkFetchResult
                {
                    compressed_chunks,
                    chunk_col.chunk_column_pos()
                }
            );
        }
    }
}