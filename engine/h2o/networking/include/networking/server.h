#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "networking_types.h"
#include "networking_utils.h"

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

        template<class MsgType>
        void send_message(ClientID client_id, const MsgType& msg);

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
        std::unordered_map<HSteamNetConnection, Client> m_steam_net_connection_to_client{};
        std::unordered_map<Client, HSteamNetConnection> m_client_to_steam_net_connection{};

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        bool m_is_active = false;

        static Server* s_callback_instance;

    };

    template<class MsgType>
    void Server::send_message(ClientID client_id, const MsgType& msg)
    {
        assert(m_is_active);

        // Prefix the message
        std::vector<u8> buffer{};
        net_utils::serialize(msg, buffer);

        // Slow and ugly, potentially not portable
        // Will work for now :)
        const MsgID id = MsgType::message_id;
        buffer.insert(buffer.cbegin(), sizeof(id), 0);
        memcpy(buffer.data(), &id, sizeof(id));

//        m_interface->SendMessageToConnection(
//            m_connection,
//            buffer.data(),
//            buffer.size(),
//            k_nSteamNetworkingSend_Reliable,
//            nullptr);
    }
}