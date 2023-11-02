#pragma once

#include "core/events.h"
#include "core/log.h"
#include "core/tickable.h"
#include "networking/networking_types.h"
#include "networking/networking_utils.h"

#include <set>
#include <steam/isteamnetworkingsockets.h>

namespace h2o
{
    class NetPeer : public Tickable
    {
    public:

        explicit NetPeer(Tickable* owner);
        NetPeer(const NetPeer&) = delete;
        ~NetPeer() override = default;

        /**
         * Send a message to this peer's connection
         *
         * @tparam MsgType The message's type, must be serializable by bitsery
         * @param msg The message to send
         */
        template<class MsgType>
        void send_message(PeerID peer_id, const MsgType& msg);

        /**
         * Bind the lambda to execute when a message of MsgType is received.
         *
         * @tparam MsgType The message's type
         * @param event_handle The event handle associated with the callback
         * @param callback The lambda to call when receiving this message
         */
        template<class MsgType>
        void handle_message(
            EventHandle& event_handle,
            const MsgReceivedEventCallback<MsgType>& callback);

        [[nodiscard]] virtual const std::set<PeerID>& peers() const = 0;

        /**
         * Disconnect a client or close a server
         */
        virtual void stop(bool unregister_from_module /* = true */);

        // Tickable interface
        void update(f32 delta_time) override;

    protected:

        void create_interface();

        // Start polling for messages and state changes
        void start_polling_messages();

        // Override these :)
        virtual void send_message_raw(PeerID client_id, void* data, u32 size) const = 0;
        [[nodiscard]] virtual i32  poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages) = 0;
        [[nodiscard]] virtual bool can_send_messages() const = 0;
        virtual void on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info) = 0;

        [[nodiscard]] Event<ReceivedMessageEvent>* get_msg_event(MsgID id);

        static void connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info);

    private:

        void register_peer();
        void unregister_peer();

        void poll_incoming_messages();
        void poll_connection_state_changes();

    protected:

        ISteamNetworkingSockets* m_interface = nullptr;

    private:

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        static NetPeer* s_callback_instance;

    };

    template<class MsgType>
    void NetPeer::send_message(PeerID peer_id, const MsgType& msg)
    {
        assert(can_send_messages());

        // Prefix the message
        std::vector<u8> buffer{};
        net_utils::serialize(msg, buffer);

        // Slow and ugly, potentially not portable
        // Will work for now :)
        const MsgID id = MsgType::message_id;
        buffer.insert(buffer.cbegin(), sizeof(id), 0);
        memcpy(buffer.data(), &id, sizeof(id));

        send_message_raw(peer_id, buffer.data(), buffer.size());
    }

    template<class MsgType>
    void NetPeer::handle_message(
        EventHandle& event_handle,
        const MsgReceivedEventCallback <MsgType>& callback)
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