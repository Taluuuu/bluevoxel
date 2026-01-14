#pragma once

#include "core/tickable.h"
#include "net_peer.h"

#include <steam/isteamnetworkingsockets.h>

namespace h2o
{
    class NetPeer_Online
        : public Tickable
        , public INetPeer
    {
    public:

        explicit NetPeer_Online(Tickable* owner);
        NetPeer_Online(const NetPeer_Online&) = delete;
        ~NetPeer_Online() override = default;

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
        [[nodiscard]] virtual std::optional<PeerID> handle_to_peer_id(HSteamNetConnection handle) const { return 0; }

        [[nodiscard]] Event<ReceivedMessageEvent>* get_msg_event(MsgID id);

        static void connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info);

        // INetPeer interface
        void handle_message_internal(MsgID msg_id, EventHandle& event_handle, const ReceivedMessageLambda& event_lambda) override;


    protected:

        ISteamNetworkingSockets* m_interface = nullptr;

    private:

        void poll_incoming_messages();
        void poll_connection_state_changes();

    private:

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        static NetPeer_Online* s_callback_instance;

    };
}