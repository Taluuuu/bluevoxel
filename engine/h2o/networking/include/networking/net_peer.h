#pragma once

#include "core/events.h"
#include "core/log.h"
#include "core/tickable.h"
#include "networking/networking_types.h"
#include "networking/networking_utils.h"

#include <set>
#include <steam/isteamnetworkingsockets.h>
#include <unordered_set>

namespace h2o
{
    class INetPeer
    {
    public:

        virtual ~INetPeer() = default;

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

        [[nodiscard]] virtual const std::unordered_set<PeerID>& peers() const = 0;

    protected:

        using ReceivedMessageLambda = std::function<void(const ReceivedMessageEvent& event)>;

        virtual void send_message_internal(PeerID client_id, const void* data, size_t size) = 0;
        virtual void handle_message_internal(MsgID msg_id, EventHandle& event_handle, const ReceivedMessageLambda& event_lambda) = 0;

    };

    // TODO: Rename this NetPeer_Online and create a NetPeer_Offline
    class NetPeer
        : public Tickable
        , public INetPeer
    {
    public:

        explicit NetPeer(Tickable* owner);
        NetPeer(const NetPeer&) = delete;
        ~NetPeer() override = default;

        /**
         * Disconnect a client or close a server
         */
        virtual void stop();

        // Tickable interface
        void update(f32 delta_time) override;

    protected:

        void create_interface();

        // Start polling for messages and state changes
        void start_polling_messages();

        // Override these :)
        [[nodiscard]] virtual i32 poll_messages(ISteamNetworkingMessage** out_messages, i32 max_messages) = 0;
        [[nodiscard]] virtual bool can_send_messages() const = 0;
        virtual void on_connection_status_changed(const SteamNetConnectionStatusChangedCallback_t& info) = 0;

        [[nodiscard]] Event<ReceivedMessageEvent>* get_msg_event(MsgID id);

        static void connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info);

    protected:

        // INetPeer interface
        void handle_message_internal(MsgID msg_id, EventHandle& event_handle, const ReceivedMessageLambda& event_lambda) override;

    protected:

        ISteamNetworkingSockets* m_interface = nullptr;

    private:

        void poll_incoming_messages();
        void poll_connection_state_changes();

    private:

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        static NetPeer* s_callback_instance;

    };

    template<class MsgType>
    void INetPeer::send_message(PeerID peer_id, const MsgType& msg)
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

        send_message_internal(peer_id, buffer.data(), buffer.size());
    }

    template<class MsgType>
    void INetPeer::handle_message(
        EventHandle& event_handle,
        const MsgReceivedEventCallback<MsgType>& callback)
    {
        const auto event_lambda =
            [callback](const ReceivedMessageEvent& event)
            {
                MsgType deserialized_msg;
                if (!net_utils::deserialize(event.msg, deserialized_msg))
                    return;

                callback(event.client_id, deserialized_msg);
            };

        handle_message_internal(MsgType::message_id, event_handle, event_lambda);
    }
}