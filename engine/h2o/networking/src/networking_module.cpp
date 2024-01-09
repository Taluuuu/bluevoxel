#include "networking/networking_module.h"

#include "core/log.h"
#include "networking/client.h"

namespace h2o
{
    static void debug_output(ESteamNetworkingSocketsDebugOutputType type, const char* msg)
    {
        switch (type)
        {
        case k_ESteamNetworkingSocketsDebugOutputType_Msg:
            log::info("GameNetworkingSockets Debug: {}", msg);
            break;

        default:
            break;
        }
    }

    bool NetworkingModule::init(Engine& engine)
    {
        SteamDatagramErrMsg err_msg;
        if (!GameNetworkingSockets_Init(nullptr, err_msg))
        {
            log::error("Failed to init GameNetworkingSockets: '{}'", err_msg);
            return false;
        }

        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, debug_output);

        return true;
    }

    void NetworkingModule::cleanup()
    {
        GameNetworkingSockets_Kill();
    }

    std::vector<std::type_index> h2o::NetworkingModule::dependencies() const
    {
        return {};
    }
}