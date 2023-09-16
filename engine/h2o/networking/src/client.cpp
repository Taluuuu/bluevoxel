#include "networking/client.h"

#include "core/engine.h"
#include "networking/networking_module.h"

namespace h2o
{
    Client* Client::s_callback_instance = nullptr;

    Client::Client()
        : m_networking_module(g_engine->get_module<NetworkingModule>())
    {
        assert(m_networking_module);
    }

    Client::~Client()
    {
        disconnect();
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

        m_networking_module->register_client(*this);

        set_tick_phases(TickPhase::Update);

        return true;
    }

    void Client::disconnect(bool unregister_from_module)
    {
        if (m_connection_state == ConnectionState::Disconnected)
            return;

        m_connection_state = ConnectionState::Disconnected;

        m_interface->CloseConnection(m_connection, 0, nullptr, false);
        m_connection = k_HSteamNetConnection_Invalid;

        set_tick_phases({});

        if (unregister_from_module)
            m_networking_module->unregister_client(*this);
    }

    void Client::update(f32 delta_time)
    {
        if (m_connection_state != ConnectionState::Disconnected)
        {
            poll_incoming_messages();
            poll_connection_state_changes();
        }
    }

    void Client::poll_incoming_messages()
    {
        ISteamNetworkingMessage* incoming_messages { nullptr };
        const i32 num_msgs = m_interface->ReceiveMessagesOnConnection(m_connection, &incoming_messages, INT_MAX);

        if (num_msgs < 0)
        {
            log::error("Error checking for messages.");
            return;
        }

        for (i32 i = 0; i < num_msgs; i++)
        {
            ISteamNetworkingMessage* msg = incoming_messages + i;

            // ...

            msg->Release();
        }
    }

    void Client::poll_connection_state_changes()
    {
        s_callback_instance = this;
        m_interface->RunCallbacks();
    }

    void Client::connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info)
    {
        assert(s_callback_instance);
        s_callback_instance->on_connection_status_changed(info);
    }

    void Client::on_connection_status_changed(SteamNetConnectionStatusChangedCallback_t* info)
    {
        if (info->m_hConn != m_connection && m_connection == k_HSteamNetConnection_Invalid)
            return;

        switch (info->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections. You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            m_connection_state = ConnectionState::Disconnected;

            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
            {
                log::info("Could not connect to server: {}", info->m_info.m_szEndDebug);
            }
            else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                log::info("Lost contact with server: {}", info->m_info.m_szEndDebug);
            }
            else
            {
                log::info("Disconnected from server: {}", info->m_info.m_szEndDebug);
            }

            m_interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            m_connection = k_HSteamNetConnection_Invalid;
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            m_connection_state = ConnectionState::Connecting;
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            m_connection_state = ConnectionState::Connected;
            log::info("Connected to server.");
            break;

        default:
            break;
        }
    }
}