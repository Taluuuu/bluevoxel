#include "bluevoxel_server_module.h"

#include "core/log.h"
#include "networking/networking_module.h"
#include "voxel_server/voxel_server_module.h"

namespace bluevoxel
{
    bool BlueVoxelServerModule::init(h2o::Engine& engine)
    {
        if (!m_server.start(1338))
            return false;

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