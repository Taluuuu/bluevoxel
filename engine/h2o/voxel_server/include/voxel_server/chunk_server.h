#pragma once

#include "core/data_structures/thread_safe_priority_queue.h"
#include "core/events.h"
#include "core/types.h"
#include "networking/networking_types.h"
#include "server_chunk_manager.h"
#include "voxel/block_container_interface.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_generators/chunk_generator_base.h"
#include "voxel/voxel_net_messages.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace h2o
{
    class ChunkGenerator_Base;
    class Server;

    struct ChunkFetchRequest
    {
        std::set<ClientID> requesting_clients{};
        v2i chunk_col_pos{};
        bool is_generating = false;
    };

    struct ChunkGenRequest
    {
        std::shared_ptr<ChunkFetchRequest> fetch_request{};
        ChunkRegion gen_region;
        i32 gen_stage = 0;
        f32 distance = 0.0f;

        [[nodiscard]] bool operator<(const ChunkGenRequest& other) const
        { return distance < other.distance; }
    };

    // This class will be adapted to be hosted on a server.
    class ChunkServer : public IBlockContainer
    {
    public:

        explicit ChunkServer(Server& server);
        ~ChunkServer() override;

        void start();
        void stop();

        [[nodiscard]] bool is_running() const;

        void set_chunk_generator(std::unique_ptr<ChunkGenerator_Base>&& chunk_generator);

        // IBlockContainer interface
        [[nodiscard]] Chunk* get_chunk_at(const v3i& chunk_pos) override;
        [[nodiscard]] const Chunk* get_chunk_at(const v3i& chunk_pos) const override;

    protected:

        void run();

        void request_chunk_generations();
        void load_requested_chunks();

        // Networking
        void on_received_chunk_fetch_requests(
            ClientID client_id,
            const net_msg::ChunkFetchRequest& chunk_fetch_request);

        void on_received_block_place_request(
            ClientID request_sender,
            const net_msg::BlockPlaceRequest& block_place_request);

        // TODO: chunk_col sould be const
        void send_chunk_column(ChunkColumn& chunk_col, const std::set<ClientID>& client_ids) const;

    private:

        // Networking
        Server* const m_server = nullptr;
        EventHandle m_received_chunk_request_handle{};
        EventHandle m_received_block_place_request_handle{};

        // Storage
        ServerChunkManager m_chunk_mgr{};

        // Generation
        std::unique_ptr<ChunkGenerator_Base> m_chunk_generator{};

        std::mutex m_chunk_fetch_requests_mutex{};
        std::vector<std::shared_ptr<ChunkFetchRequest>> m_chunk_fetch_requests{};

        std::mutex m_chunk_gen_dequeue_mutex{};
        std::deque<ChunkGenRequest> m_chunk_gen_deque{};

        // Threading
        std::thread m_thread{};
        std::atomic_bool m_should_stop { true };

    };
}