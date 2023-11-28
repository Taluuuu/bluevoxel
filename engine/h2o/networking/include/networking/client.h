#pragma once

#include "core/events.h"
#include "networking_types.h"
#include "networking_utils.h"
#include "net_peer.h"

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/traits/vector.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <vector>

namespace h2o
{
    class NetworkingModule;

    class Client : public NetPeer
    {
    public:

        explicit Client(Tickable* owner);
        Client(const Client&) = delete;
        ~Client() override;

        bool connect(const std::string& hostname, u16 port);

        [[nodiscard]] ConnectionState connection_state() const { return m_connection_state; }
        [[nodiscard]] bool is_connected() const { return m_connection_state == ConnectionState::Connected; }

    public:

        struct ConnectionEvent{};
        Event<ConnectionEvent> on_connected_to_server{};
        Event<ConnectionEvent> on_disconnected_from_server{};

        // NetPeer interface
        [[nodiscard]] const std::set<PeerID>& peers() const override;
        void stop() final;
    protected:
        void send_message_raw(PeerID client_id, void* data, u32 size) const override;
        [[nodiscard]] i32  poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages) override;
        [[nodiscard]] bool can_send_messages() const override;
        void on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info) override;

        void on_connected();

    private:

        ConnectionState m_connection_state = ConnectionState::Disconnected;

        HSteamNetConnection m_connection{};

    };
}