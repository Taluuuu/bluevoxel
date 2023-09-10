#include "bluevoxel_client_module.h"

#include "voxel_client/voxel_client_module.h"
#include "networking/networking_module.h"

namespace bluevoxel
{
    bool BlueVoxelClientModule::init(h2o::Engine& engine)
    {
        if (!m_client.connect("::1", 1338))
            return false;

        return true;
    }

    void BlueVoxelClientModule::cleanup()
    {
        m_client.disconnect();
    }

    std::vector<std::type_index> BlueVoxelClientModule::dependencies() const
    {
        return {
            typeid(h2o::NetworkingModule),
            typeid(h2o::VoxelClientModule)
        };
    }
}