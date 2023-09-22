#include "bluevoxel_server_module.h"

#include "core/engine.h"
#include "networking/message_ids.h"
#include "networking/networking_module.h"
#include "networking/networking_utils.h"
#include "networking/test_message.h"
#include "voxel_server/chunk_server.h"
#include "voxel_server/voxel_server_module.h"

#include <vector>

namespace bluevoxel
{
    bool BlueVoxelServerModule::init(h2o::Engine& engine)
    {
        if (!m_server.start(1338))
            return false;

        m_chunk_server = std::make_unique<h2o::ChunkServer>(m_server);
        m_chunk_server->start();

        m_server.handle_message<h2o::TestMessage>(m_received_msg_handle,
            [](h2o::ClientID client_id, const auto& test_msg)
            {
                h2o::log::info("Received data: {} - {}", test_msg.num, test_msg.text);
            }
        );

        return true;
    }

    void BlueVoxelServerModule::cleanup()
    {
        m_chunk_server->stop();
        m_chunk_server.reset();

        m_server.stop(true);
    }

    std::vector<std::type_index> bluevoxel::BlueVoxelServerModule::dependencies() const
    {
        return {
            typeid(h2o::NetworkingModule),
            typeid(h2o::VoxelServerModule)
        };
    }
}