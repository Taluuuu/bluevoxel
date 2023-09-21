#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "networking_types.h"
#include "networking_utils.h"

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

    class Client : public Tickable
    {
    public:

        Client() = default;
        Client(const Client&) = delete;
        ~Client() override;

        bool connect(const std::string& hostname, u16 port);
        void disconnect(bool unregister_from_module = true);

        template<class MsgType>
        void send_message(const MsgType& msg);

        [[nodiscard]] Event<ReceivedMessageEvent>& handle_msg(MsgID id);
        [[nodiscard]] Event<ReceivedMessageEvent>* get_msg_event(MsgID id);

        [[nodiscard]] ConnectionState connection_state() const { return m_connection_state; }
        [[nodiscard]] bool is_connected() const { return m_connection_state == ConnectionState::Connected; }

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

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

        static Client* s_callback_instance;

    };

    template<class MsgType>
    void Client::send_message(const MsgType& msg)
    {
        assert(m_connection_state == ConnectionState::Connected);

        // Prefix the message
        std::vector<u8> buffer{};
        net_utils::serialize(msg, buffer);

        // Slow and ugly, potentially not portable
        // Will work for now :)
        const MsgID id = MsgType::message_id;
        buffer.insert(buffer.cbegin(), sizeof(id), 0);
        memcpy(buffer.data(), &id, sizeof(id));

        m_interface->SendMessageToConnection(
            m_connection,
            buffer.data(),
            buffer.size(),
            k_nSteamNetworkingSend_Reliable,
            nullptr);
    }
}