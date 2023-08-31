#pragma once

#include "chunk_column.h"
#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <queue>
#include <thread>
#include <unordered_map>

namespace h2o
{
    using ClientID = u32;

    // This will be sent to the server by the client.
    struct VoxelClientInput
    {
        v2i position{};
        i32 view_distance{};
    };

    // This is the data the server will send back to the client.
    struct VoxelClientOutput
    {
        // Chunks that have been loaded by the chunk manager awaiting to be
        // received by the client.
        std::queue<WeakHandle<ChunkColumn>> pending_chunks{};
    };

    // This class will be adapted to be hosted on a server.
    class ChunkManager
    {
    public:

        ChunkManager() = default;
        ~ChunkManager();

        void start();
        void stop();

        void register_client(ClientID client_id, const VoxelClientInput& client_input);
        void unregister_client(ClientID client_id);

        [[nodiscard]] VoxelClientInput* client_input(ClientID client_id) ;
        [[nodiscard]] VoxelClientOutput* client_outputs(ClientID client_id);

    protected:

        void run();

        void request_chunk_loads();
        void load_requested_chunks();

    private:

        // Networking
        std::mutex m_client_inputs_mutex{};
        std::unordered_map<ClientID, VoxelClientInput> m_client_inputs{};
        std::mutex m_client_outputs_mutex{};
        std::unordered_map<ClientID, VoxelClientOutput> m_client_outputs{};

        // Storage
        std::mutex m_loaded_chunks_mutex{};
        std::unordered_map<v2i, OwningHandle<ChunkColumn>> m_loaded_chunks{};

        // Threading
        std::thread m_thread{};
        std::atomic_bool m_should_stop { true };

    };
}