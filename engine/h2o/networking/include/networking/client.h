#pragma once

#include "core/tickable.h"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace h2o
{
    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Connected
    };

    class Client : public Tickable
    {
    public:

        Client() = default;
        ~Client() override;

        bool connect(const std::string& hostname, u16 port);
        void disconnect(bool unregister_from_module = true);

        void send_message(const void* msg, size_t msg_len);

        [[nodiscard]] ConnectionState connection_state() const { return m_connection_state; }

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        void poll_incoming_messages();
        void poll_connection_state_changes();

        static void connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info);
        void on_connection_status_changed(SteamNetConnectionStatusChangedCallback_t* info);

    private:

        ConnectionState m_connection_state = ConnectionState::Disconnected;

        ISteamNetworkingSockets* m_interface { nullptr };
        HSteamNetConnection m_connection{};

        static Client* s_callback_instance;

    };
}