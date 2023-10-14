#include "networking/server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/networking_module.h"

namespace h2o
{
    Server::~Server()
    {
        stop(true);
    }

    bool Server::start(u16 port)
    {
        create_interface();

        // Create listen socket
        SteamNetworkingIPAddr server_local_addr{};
        server_local_addr.Clear();
        server_local_addr.m_port = port;

        std::array<SteamNetworkingConfigValue_t, 2> opts{};
        opts[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)connection_status_changed_callback);
        opts[1].SetInt32(k_ESteamNetworkingConfig_SendBufferSize, 10*1024*1024);

        m_listen_socket = m_interface->CreateListenSocketIP(server_local_addr, opts.size(), opts.data());
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

        set_tick_phases(TickPhase::TickPhase_Update);

        return true;
    }

    void Server::stop(bool unregister_from_module)
    {
        for (const u32 client_id : m_client_ids)
            m_interface->CloseConnection(client_id, 0, "Server is shutting down.", true);

        m_client_ids.clear();

        m_interface->CloseListenSocket(m_listen_socket);
        m_listen_socket = k_HSteamListenSocket_Invalid;

        m_interface->DestroyPollGroup(m_poll_group);
        m_poll_group = k_HSteamNetPollGroup_Invalid;

        m_is_active = false;

        NetPeer::stop(unregister_from_module);
    }

    void Server::send_message_raw(ClientID client_id, void* data, u32 size) const
    {
        if (m_client_ids.contains(client_id))
        {
            m_interface->SendMessageToConnection(
                client_id,
                data,
                size,
                k_nSteamNetworkingSend_Reliable,
                nullptr);
        }
    }

    i32 Server::poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages)
    {
        return m_interface->ReceiveMessagesOnPollGroup(
            m_poll_group,
            out_messages,
            max_messages);
    }

    bool Server::can_send_messages() const
    {
        return m_is_active;
    }

    void Server::on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info)
    {
        switch (info.m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_None:
            // This is called when we destroy connections, we can ignore.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            // Ignore if they were not previously connected. (If they disconnected
            // before we accepted the connection.)
            if (info.m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {
                m_client_ids.erase(info.m_hConn);

                // Handle disconnect...
                log::info("Client disconnected.");
            }
            else
            {
                assert(info.m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
            }

            m_interface->CloseConnection(info.m_hConn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            log::info("Connection request from {}.", info.m_info.m_szConnectionDescription);

            if (m_interface->AcceptConnection(info.m_hConn) != k_EResultOK)
            {
                m_interface->CloseConnection(info.m_hConn, 0, nullptr, false);
                log::info("Can't accept connection. (It was already closed?)");
                break;
            }

            if (!m_interface->SetConnectionPollGroup(info.m_hConn, m_poll_group))
            {
                m_interface->CloseConnection(info.m_hConn, 0, nullptr, false);
                log::info("Failed to set poll group?");
                break;
            }

            m_client_ids.insert(info.m_hConn);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            on_player_joined.broadcast({ info.m_hConn });
            break;

        default:
            break;
        }
    }
}