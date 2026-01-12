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

        constexpr i32 min_send_rate = 1 * 1024 * 1024;
        constexpr i32 max_send_rate = 100 * 1024 * 1024;
        SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendRateMin, min_send_rate);
        SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendRateMax, max_send_rate);

        return true;
    }

    void NetworkingModule::cleanup()
    {
        GameNetworkingSockets_Kill();
    }

    std::vector<std::type_index> NetworkingModule::dependencies() const
    {
        return {};
    }
}