#include "voxel/chunk_manager.h"

#include "core/log.h"

namespace h2o
{
    ChunkManager::ChunkManager()
    {

    }

    void ChunkManager::register_client(u32 client_id, const VoxelClient& chunk_loader)
    {
        const auto it = m_voxel_clients.find(client_id);
        if (it == m_voxel_clients.end())
        {
            m_voxel_clients[client_id] = chunk_loader;
        }
        else
        {
            log::warn("Trying to register two chunk loaders with the same id: {}", client_id);
        }
    }

    void ChunkManager::unregister_client(u32 client_id)
    {
        if (m_voxel_clients.erase(client_id) == 0)
        {
            log::warn("Trying to remove a chunk loader with id '{}' when none was found.", client_id);
        }
    }

    std::queue<WeakHandle<ChunkColumn>>& ChunkManager::loaded_chunk_queue(u32 client_id)
    {
        static std::queue<WeakHandle<ChunkColumn>> empty{};
//        if (m_loaded_chunk_queues)
    }
}