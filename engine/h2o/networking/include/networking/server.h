#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "networking_types.h"
#include "networking_utils.h"

#include <map>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <unordered_map>
#include <set>

namespace h2o
{
    template<class T>
    using MsgReceivedEventCallback = std::function<void(ClientID, const T&)>;

    class Server : public Tickable
    {
    public:

        Server() = default;
        Server(const Server&) = delete;
        ~Server() override;

        bool start(u16 port);
        void stop(bool unregister_from_module = false);

        template<class MsgType>
        void send_message(ClientID client_id, const MsgType& msg);

        template<class MsgType>
        void handle_msg(
            EventHandle& event_handle,
            const MsgReceivedEventCallback<MsgType>& callback);

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

        std::set<ClientID> m_client_ids{};

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        bool m_is_active = false;

        static Server* s_callback_instance;

    };

    template<class MsgType>
    void Server::send_message(ClientID client_id, const MsgType& msg)
    {
        assert(m_is_active);
        assert(m_client_ids.contains(client_id));

        // Prefix the message
        std::vector<u8> buffer{};
        net_utils::serialize(msg, buffer);

        // Slow and ugly, potentially not portable
        // Will work for now :)
        const MsgID id = MsgType::message_id;
        buffer.insert(buffer.cbegin(), sizeof(id), 0);
        memcpy(buffer.data(), &id, sizeof(id));

        m_interface->SendMessageToConnection(
            client_id,
            buffer.data(),
            buffer.size(),
            k_nSteamNetworkingSend_Reliable,
            nullptr);
    }

    template<class MsgType>
    void Server::handle_msg(
        EventHandle& event_handle,
        const MsgReceivedEventCallback<MsgType>& callback)
    {
        const MsgID id = MsgType::message_id;

        const auto it = m_message_received_events.find(id);
        assert(it == m_message_received_events.end()); // Is msg handled multiple times ?

        const auto event_lambda =
            [callback](const ReceivedMessageEvent& event)
            {
                MsgType deserialized_msg;
                if (!net_utils::deserialize(event.msg, deserialized_msg))
                    return;

                callback(event.client_id, deserialized_msg);
            };

        m_message_received_events[id].add_listener(event_handle, event_lambda);
    }
}