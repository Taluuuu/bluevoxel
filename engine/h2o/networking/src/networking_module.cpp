#include "networking/networking_module.h"

#include "core/log.h"

#include <enet.h>

namespace h2o
{
    bool NetworkingModule::init(Engine& engine)
    {
        if (enet_initialize() != 0)
        {
            log::error("Failed to initialize ENet.");
            return false;
        }

        return true;
    }

    void NetworkingModule::cleanup()
    {
        enet_deinitialize();
    }

    std::vector<std::type_index> h2o::NetworkingModule::dependencies() const
    {
        return {};
    }
}