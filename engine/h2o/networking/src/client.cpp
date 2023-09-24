#include "networking/client.h"

#include "core/engine.h"
#include "networking/networking_module.h"

namespace h2o
{
    Client::~Client()
    {
        stop(true);
    }

    bool Client::connect(const std::string& hostname, u16 port)
    {
        if (m_connection_state != ConnectionState::Disconnected)
            return true;

        m_interface = SteamNetworkingSockets();

        // TODO: Check if this is necessary
        SteamNetworkingIPAddr address{}; address.Clear();
        if (!address.ParseString(hostname.c_str()))
        {
            log::error("Invalid IP address: {}:{}", hostname, port);
            return false;
        }
        address.m_port = port;

        m_connection_state = ConnectionState::Connecting;
        log::info("Connecting to server at {}:{}", hostname, port);

        SteamNetworkingConfigValue_t opt{};
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)connection_status_changed_callback);
        m_connection = m_interface->ConnectByIPAddress(address, 1, &opt);
        if (m_connection == k_HSteamNetConnection_Invalid)
        {
            log::error("Failed to create connection.");
            return false;
        }

        start_polling_messages();

        return true;
    }

    void Client::stop(bool unregister_from_module)
    {
        if (m_connection_state == ConnectionState::Disconnected)
            return;

        m_connection_state = ConnectionState::Disconnected;

        m_interface->CloseConnection(m_connection, 0, nullptr, false);
        m_connection = k_HSteamNetConnection_Invalid;

        set_tick_phases({});

        NetPeer::stop(unregister_from_module);
    }

    void Client::send_message_raw(ClientID client_id, void* data, u32 size) const
    {
        m_interface->SendMessageToConnection(
            m_connection,
            data,
            size,
            k_nSteamNetworkingSend_Reliable,
            nullptr);
    }

    i32 Client::poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages)
    {
        return m_interface->ReceiveMessagesOnConnection(
            m_connection,
            out_messages,
            max_messages);
    }

    bool Client::can_send_messages() const
    {
        return is_connected();
    }

    void Client::on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info)
    {
        if (info.m_hConn != m_connection && m_connection == k_HSteamNetConnection_Invalid)
            return;

        switch (info.m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections. You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            stop(true);
            m_connection_state = ConnectionState::Disconnected;

            if (info.m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
            {
                log::info("Could not connect to server: {}", info.m_info.m_szEndDebug);
            }
            else if (info.m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                log::info("Lost contact with server: {}", info.m_info.m_szEndDebug);
            }
            else
            {
                log::info("Disconnected from server: {}", info.m_info.m_szEndDebug);
            }

            m_interface->CloseConnection(info.m_hConn, 0, nullptr, false);
            m_connection = k_HSteamNetConnection_Invalid;
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            m_connection_state = ConnectionState::Connecting;
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            log::info("Connected to server.");
            m_connection_state = ConnectionState::Connected;
            on_connected_to_server.broadcast({});
            break;

        default:
            break;
        }
    }
}