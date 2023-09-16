#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "networking_types.h"

#include <map>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <unordered_map>

namespace h2o
{
    class Server : public Tickable
    {
    public:

        Server() = default;
        ~Server() override;

        bool start(u16 port);
        void stop(bool unregister_from_module = false);

        [[nodiscard]] Event<ReceivedMessageEvent>& handle_msg(MsgID id);
        [[nodiscard]] Event<ReceivedMessageEvent>* get_msg_event(MsgID id);

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        void poll_incoming_messages();
        void poll_connection_state_changes();

        static void connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info);
        void on_connection_status_changed(SteamNetConnectionStatusChangedCallback_t* info);

    private:

        ISteamNetworkingSockets* m_interface { nullptr };
        HSteamListenSocket m_listen_socket{};
        HSteamNetPollGroup m_poll_group{};

        struct Client { i32 id{}; };
        std::map<HSteamNetConnection, Client> m_client_map{};

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        bool m_is_active = false;

        static Server* s_callback_instance;

    };
}