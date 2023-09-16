#include "bluevoxel_server_module.h"

#include "core/engine.h"
#include "networking/networking_module.h"
#include "networking/networking_utils.h"
#include "voxel_server/voxel_server_module.h"

#include <vector>

namespace bluevoxel
{
    bool BlueVoxelServerModule::init(h2o::Engine& engine)
    {
        if (!m_server.start(1338))
            return false;

        m_server.handle_msg(312).add_listener(m_received_msg_handle,
            [](const h2o::ReceivedMessageEvent& event)
            {
                h2o::TestMessage test_msg;
                if (h2o::net_utils::deserialize(event.msg, test_msg))
                    h2o::log::info("Received data: {} - {}", test_msg.num, test_msg.text);
            });

        return true;
    }

    void BlueVoxelServerModule::cleanup()
    {
        m_server.stop();
    }

    std::vector<std::type_index> bluevoxel::BlueVoxelServerModule::dependencies() const
    {
        return {
            typeid(h2o::NetworkingModule),
            typeid(h2o::VoxelServerModule)
        };
    }
}