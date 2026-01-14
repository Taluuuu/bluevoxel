#include "networking/server.h"

#include "welcome_msg.h"
#include "core/log.h"
#include "networking/networking_module.h"

namespace h2o
{
    Server::Server(Tickable* owner)
        : NetPeer_Online(owner)
    {}

    Server::~Server()
    {
        stop();
    }

    bool Server::start(u16 port, const bool allow_only_localhost)
    {
        m_allow_only_localhost = allow_only_localhost;

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

        set_tick_phases(TickPhase::Update);

        return true;
    }

    std::span<const PeerID> Server::peers() const
    {
        return m_peer_id_manager.peers();
    }

    void Server::stop()
    {
        if (!m_is_active)
            return;

        for (const PeerID peer_id : m_peer_id_manager.peers())
        {
            if (const auto handle = m_peer_id_manager.get_handle(peer_id))
                m_interface->CloseConnection(*handle, 0, "Server is shutting down.", true);
        }
        m_peer_id_manager.clear();

        m_interface->CloseListenSocket(m_listen_socket);
        m_listen_socket = k_HSteamListenSocket_Invalid;

        m_interface->DestroyPollGroup(m_poll_group);
        m_poll_group = k_HSteamNetPollGroup_Invalid;

        m_is_active = false;

        NetPeer_Online::stop();
    }

    void Server::send_message_internal(const PeerID client_id, const void* data, const size_t size)
    {
        if (const auto handle = m_peer_id_manager.get_handle(client_id))
        {
            m_interface->SendMessageToConnection(
                *handle,
                data,
                size,
                k_nSteamNetworkingSend_Reliable,
                nullptr);
        }
    }

    i32 Server::poll_messages(ISteamNetworkingMessage** out_messages, const i32 max_messages)
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
                log::info("Client disconnected.");

                // Handle disconnect...
                m_peer_id_manager.remove_peer(info.m_hConn);

                on_player_left.broadcast({info.m_hConn});
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

            if (m_allow_only_localhost && !info.m_info.m_addrRemote.IsLocalHost())
            {
                m_interface->CloseConnection(info.m_hConn, 0, nullptr, false);
                log::info("Can't accept connection. Localhost only is enabled, and the client is not local.");
                break;
            }

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

            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            if (const auto peer_id = m_peer_id_manager.add_peer(info.m_hConn))
            {
                send_message<WelcomeMsg>(*peer_id, { *peer_id });
                on_player_joined.broadcast({ *peer_id });
            }
            else
            {
                m_interface->CloseConnection(info.m_hConn, 0, nullptr, false);
            }
            break;

        default:
            break;
        }
    }

    std::optional<PeerID> Server::handle_to_peer_id(const HSteamNetConnection handle) const
    {
        return m_peer_id_manager.get_peer_id(handle);
    }
}
