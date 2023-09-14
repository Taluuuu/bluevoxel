#include "networking/server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/networking_module.h"

namespace h2o
{
    Server* Server::s_callback_instance = nullptr;

    Server::~Server()
    {
        stop();
    }

    bool Server::start(u16 port)
    {
        m_interface = SteamNetworkingSockets();

        // Create listen socket
        SteamNetworkingIPAddr server_local_addr{};
        server_local_addr.Clear();
        server_local_addr.m_port = port;

        SteamNetworkingConfigValue_t opt{};
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)connection_status_changed_callback);

        m_listen_socket = m_interface->CreateListenSocketIP(server_local_addr, 1, &opt);
        if (m_listen_socket == k_HSteamListenSocket_Invalid)
        {
            log::error("Failed to listen on port {}", port);
            return false;
        }

        m_poll_group = m_interface->CreatePollGroup();
        if (m_poll_group == k_HSteamNetPollGroup_Invalid)
        {
            log::error("Failed to listen on port {}", port);
            return false;
        }

        log::info("Server listening on port {}", port);
        m_is_active = true;

        auto networking_module = g_engine->get_module<NetworkingModule>();
        assert(networking_module);
        networking_module->register_server(*this);

        set_tick_phases(TickPhase::Update);

        return true;
    }

    void Server::stop(bool unregister_from_module)
    {
        for (const auto& [connection, client] : m_client_map)
            m_interface->CloseConnection(connection, 0, "Server is shutting down.", true);

        m_client_map.clear();

        m_interface->CloseListenSocket(m_listen_socket);
        m_listen_socket = k_HSteamListenSocket_Invalid;

        m_interface->DestroyPollGroup(m_poll_group);
        m_poll_group = k_HSteamNetPollGroup_Invalid;

        m_is_active = false;

        if (unregister_from_module)
        {
            auto networking_module = g_engine->get_module<NetworkingModule>();
            assert(networking_module);

            networking_module->unregister_server(*this);
        }

        set_tick_phases({});
    }

    void Server::update(f32 delta_time)
    {
        if (m_is_active)
        {
            poll_incoming_messages();
            poll_connection_state_changes();
        }
    }

    void Server::poll_incoming_messages()
    {
        ISteamNetworkingMessage* incoming_messages { nullptr };
        const i32 num_msgs = m_interface->ReceiveMessagesOnPollGroup(m_poll_group, &incoming_messages, INT_MAX);

        if (num_msgs < 0)
        {
            log::error("Error checking for messages.");
            return;
        }

        for (i32 i = 0; i < num_msgs; i++)
        {
            ISteamNetworkingMessage* msg = incoming_messages + i;

            auto it_client = m_client_map.find(incoming_messages->GetConnection());

//            msg->GetData()

            msg->Release();
        }
    }

    void Server::connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info)
    {
        assert(s_callback_instance);
        s_callback_instance->on_connection_status_changed(info);
    }

    void Server::poll_connection_state_changes()
    {
        s_callback_instance = this;
        m_interface->RunCallbacks();
    }

    void Server::on_connection_status_changed(SteamNetConnectionStatusChangedCallback_t* info)
    {
        switch (info->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_None:
            // This is called when we destroy connections, we can ignore.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            // Ignore if they were not previously connected. (If they disconnected
            // before we accepted the connection.)
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {
                auto it_client = m_client_map.find(info->m_hConn);
                assert(it_client != m_client_map.end());

                // Handle disconnect...
                log::info("Client disconnected.");

                m_client_map.erase(it_client);
            }
            else
            {
                assert(info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
            }

            m_interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            assert(m_client_map.find(info->m_hConn) == m_client_map.end());

            log::info("Connection request from {}.", info->m_info.m_szConnectionDescription);

            if (m_interface->AcceptConnection(info->m_hConn) != k_EResultOK)
            {
                m_interface->CloseConnection(info->m_hConn, 0, nullptr,  false);
                log::info("Can't accept connection. (It was already closed?)");
                break;
            }

            if (!m_interface->SetConnectionPollGroup(info->m_hConn, m_poll_group))
            {
                m_interface->CloseConnection(info->m_hConn, 0, nullptr, false);
                log::info("Failed to set poll group?");
                break;
            }

            m_client_map.emplace(info->m_hConn, 0);
        }

        case k_ESteamNetworkingConnectionState_Connected:
            break;

        default:
            break;
        }
    }
}