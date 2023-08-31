#pragma once

#include "chunk_column.h"
#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <queue>
#include <unordered_map>

namespace h2o
{
    // This will be sent to the server by the client.
    struct VoxelClient
    {
        v2i position{};
        i32 view_distance{};
    };

    // This is the data the server will send back to the client.
    struct VoxelClientOutput
    {
        // Chunks that have been loaded by the chunk manager awaiting to be
        // received by the client.
        std::queue<WeakHandle<ChunkColumn>> pending_chunks;
    };

    class ChunkManager
    {
    public:

        ChunkManager();

        void register_client(u32 client_id, const VoxelClient& chunk_loader);
        void unregister_client(u32 client_id);

        std::queue< WeakHandle<ChunkColumn> >& loaded_chunk_queue(u32 client_id);

    private:


        std::unordered_map<u32, VoxelClient> m_voxel_clients{};

        std::unordered_map<v2i, OwningHandle<ChunkColumn>> m_loaded_chunks{};

    };
}