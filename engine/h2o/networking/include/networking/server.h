#pragma once

#include "core/events.h"
#include "networking_types.h"
#include "networking_utils.h"
#include "net_peer.h"

#include <map>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <unordered_map>
#include <set>

namespace h2o
{
    class Server : public NetPeer
    {
    public:

        explicit Server(Tickable* owner);
        Server(const Server&) = delete;
        ~Server() override;

        bool start(u16 port);

        // NetPeer interface
        [[nodiscard]] const std::set<PeerID>& peers() const override { return m_client_ids; }
        void stop(bool unregister_from_module) final;
    protected:
        void send_message_raw(PeerID client_id, void* data, u32 size) const override;
        [[nodiscard]] i32  poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages) override;
        [[nodiscard]] bool can_send_messages() const override;
        void on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info) override;

    public:

        struct PlayerConnectionChangedEvent { PeerID client_id = 0; };
        Event<PlayerConnectionChangedEvent> on_player_joined{};
        Event<PlayerConnectionChangedEvent> on_player_left{};

    private:

        HSteamListenSocket m_listen_socket{};
        HSteamNetPollGroup m_poll_group{};

        std::set<PeerID> m_client_ids{};

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        bool m_is_active = false;

    };
}