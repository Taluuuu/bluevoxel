#include "networking/networking_module.h"

#include "core/log.h"
#include "networking/client.h"
#include "networking/server.h"

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
        for (Server* server : m_active_servers)
            server->stop(false);

        m_active_servers.clear();

        for (Client* client : m_connected_clients)
            client->disconnect(false);

        m_connected_clients.clear();

        enet_deinitialize();
    }

    std::vector<std::type_index> h2o::NetworkingModule::dependencies() const
    {
        return {};
    }
}