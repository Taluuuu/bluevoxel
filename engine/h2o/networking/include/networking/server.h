#pragma once

#include "core/events.h"
#include "networking_types.h"
#include "networking_utils.h"
#include "net_peer_online.h"
#include "peer_id_manager.h"

#include <map>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <unordered_map>

namespace h2o
{
    class Server : public NetPeer_Online
    {
    public:

        explicit Server(Tickable* owner);
        Server(const Server&) = delete;
        ~Server() override;

        bool start(u16 port, bool allow_only_localhost = false);

        // NetPeer interface
        [[nodiscard]] std::span<const PeerID> peers() const override;
        [[nodiscard]] PeerID local_peer_id() const override { return 0; }
        [[nodiscard]] bool is_connected() const override { return m_is_active; }
        void stop() final;
    protected:
        void send_message_internal(PeerID client_id, const void* data, size_t size) override;
        [[nodiscard]] i32 poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages) override;
        [[nodiscard]] bool can_send_messages() const override;
        void on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info) override;
        [[nodiscard]] std::optional<PeerID> handle_to_peer_id(HSteamNetConnection handle) const override;

    public:

        struct PlayerConnectionChangedEvent { PeerID client_id = 0; };
        Event<PlayerConnectionChangedEvent> on_player_joined{};
        Event<PlayerConnectionChangedEvent> on_player_left{};

    private:

        HSteamListenSocket m_listen_socket{};
        HSteamNetPollGroup m_poll_group{};

        bool m_is_active = false;

        bool m_allow_only_localhost = false;

        PeerIDManager m_peer_id_manager{};

    };
}
