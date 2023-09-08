#pragma once

#include "chunk_manager.h"
#include "core/data_structures/thread_safe_priority_queue.h"
#include "core/types.h"
#include "voxel/chunk_column.h"

#include "glm/gtx/hash.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace h2o
{
    class ChunkGenerator_Base;

    using ClientID = u32;

    // This will be sent to the server by the client.
    struct VoxelClientInput
    {
        // The client's position
        v2i position{};

        // The client's requested chunks
        std::queue<v2i> load_requests{};
    };

    // This is the data the server will send back to the client.
    struct VoxelClientOutput
    {
        // Chunks that have been loaded by the chunk manager awaiting to be
        // received by the client.
        std::queue< std::shared_ptr<ChunkColumn> > loaded_chunks{};
    };

    struct VoxelClient
    {
        VoxelClientInput input{};
        VoxelClientOutput output{};

        mutable std::mutex mutex{};
    };

    struct ChunkGenRequest
    {
        ChunkRegion gen_region;
        i32 gen_stage { 0 };
        f32 distance { 0.0f };
        ClientID requester_id { 0 };

        [[nodiscard]] bool operator<(const ChunkGenRequest& other) const
        { return distance < other.distance; }
    };

    // This class will be adapted to be hosted on a server.
    class ChunkServer
    {
    public:

        ChunkServer() = default;
        ~ChunkServer();

        void start();
        void stop();

        void register_client(ClientID client_id);
        void unregister_client(ClientID client_id);

        void set_chunk_generator(std::unique_ptr<ChunkGenerator_Base>&& chunk_generator);

    protected:

        void run();

        void request_chunk_loads(std::deque<ChunkGenRequest>& chunk_gen_dequeue);
        void load_requested_chunks(std::deque<ChunkGenRequest>& chunk_gen_dequeue);

    private:

        // Networking
        mutable std::mutex m_clients_mutex{};
        std::unordered_map< ClientID, std::unique_ptr<VoxelClient> > m_clients{};

        // Storage
        ChunkManager m_chunk_mgr{};

        // Generation
        std::unique_ptr<ChunkGenerator_Base> m_chunk_generator { nullptr };

        // Threading
        std::thread m_thread{};
        std::atomic_bool m_should_stop { true };

    };
}