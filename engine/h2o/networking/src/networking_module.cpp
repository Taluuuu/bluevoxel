#include "networking/networking_module.h"

#include "core/log.h"
#include "networking/client.h"
#include "networking/server.h"

#include <steam/steamuniverse.h>

namespace h2o
{
    void NetworkingModule::register_server(Server& server)
    {
        m_active_servers.insert(&server);
    }

    void NetworkingModule::unregister_server(Server& server)
    {
        m_active_servers.erase(&server);
    }

    void NetworkingModule::register_client(Client& client)
    {
        m_connected_clients.insert(&client);
    }

    void NetworkingModule::unregister_client(Client& client)
    {
        m_connected_clients.erase(&client);
    }

    static void debug_output(ESteamNetworkingSocketsDebugOutputType type, const char* msg)
    {
        switch (type)
        {
        case k_ESteamNetworkingSocketsDebugOutputType_None:
        case k_ESteamNetworkingSocketsDebugOutputType_Bug:
        case k_ESteamNetworkingSocketsDebugOutputType_Error:
        case k_ESteamNetworkingSocketsDebugOutputType_Important:
        case k_ESteamNetworkingSocketsDebugOutputType_Warning:
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
        for (Server* server : m_active_servers)
            server->stop(false);

        m_active_servers.clear();

        for (Client* client : m_connected_clients)
            client->disconnect(false);

        m_connected_clients.clear();

        GameNetworkingSockets_Kill();
    }

    std::vector<std::type_index> h2o::NetworkingModule::dependencies() const
    {
        return {};
    }
}