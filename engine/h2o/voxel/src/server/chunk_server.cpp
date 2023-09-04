#include "voxel/server/chunk_server.h"

#include "core/log.h"

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

    void ChunkServer::register_client(ClientID client_id, const VoxelClientInput& client_input)
    {
        const auto it = m_client_inputs.find(client_id);
        if (it == m_client_inputs.end())
        {
            m_client_inputs[client_id] = client_input;
        }
        else
        {
            log::warn("Trying to register two chunk loaders with the same id: {}", client_id);
        }
    }

    void ChunkServer::unregister_client(ClientID client_id)
    {
        if (m_client_inputs.erase(client_id) == 0)
        {
            log::warn("Trying to remove a chunk loader with id '{}' when none was found.", client_id);
        }
    }

    VoxelClientInput* ChunkServer::client_input(ClientID client_id)
    {
        const auto it = m_client_inputs.find(client_id);
        return it == m_client_inputs.end() ? nullptr : &it->second;
    }

    VoxelClientOutput* ChunkServer::client_outputs(ClientID client_id)
    {
        const auto it = m_client_outputs.find(client_id);
        return it == m_client_outputs.end() ? nullptr : &it->second;
    }

    void ChunkServer::run()
    {
        while (!m_should_stop)
        {
            request_chunk_loads();
            load_requested_chunks();
        }
    }

    void ChunkServer::request_chunk_loads()
    {
//        const std::lock_guard clients_guard(m_client_inputs_mutex);
//        const std::lock_guard loaded_chunks_guard(m_chunk_mgr_mutex);
//
//        for (auto& [client_id, client_input] : m_client_inputs)
//        {
//            while (!client_input.requested_chunks.empty())
//            {
//                const v2i chunk_pos = client_input.requested_chunks.front();
//                client_input.requested_chunks.pop();
//
//                const auto chunk_it = m_loaded_chunks.find(chunk_pos);
//                if (chunk_it == m_loaded_chunks.end())
//                {
//                    // This will be where we check if the chunk can be loaded from disk.
//                    // For now, this chunk needs to be generated.
//
//                }
//                else
//                {
//
//                }
//            }

//            for (i32 i = -client_input.view_distance; i <= client_input.view_distance * 2; i++)
//            for (i32 j = -client_input.view_distance; j <= client_input.view_distance * 2; j++)
//            {
//                const v2i world_pos = v2i{ i, j } + client_input.position;
//
//                const auto chunk_it = m_loaded_chunks.find(world_pos);
//                if (chunk_it == m_loaded_chunks.end())
//                {
//                    // This will be where we check if the chunk can be loaded from disk.
//                    // For now, this chunk needs to be generated.
//                }
//                else
//                {
//
//                }
//            }
//        }
    }

    void ChunkServer::load_requested_chunks()
    {

    }
}