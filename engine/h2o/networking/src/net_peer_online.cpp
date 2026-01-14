#include "networking/net_peer_online.h"

#include "core/log.h"

namespace h2o
{
    NetPeer_Online* NetPeer_Online::s_callback_instance = nullptr;

    NetPeer_Online::NetPeer_Online(Tickable* owner)
        : Tickable(owner)
    {}

    void NetPeer_Online::stop()
    {
        set_tick_phases({});
    }

    void NetPeer_Online::update(f32 delta_time)
    {
        poll_incoming_messages();
        poll_connection_state_changes();
    }

    void NetPeer_Online::create_interface()
    {
        if (!m_interface)
            m_interface = SteamNetworkingSockets();
    }

    void NetPeer_Online::start_polling_messages()
    {
        set_tick_phases(TickPhase::Update);
    }

    Event<ReceivedMessageEvent>* NetPeer_Online::get_msg_event(MsgID id)
    {
        const auto it = m_message_received_events.find(id);
        return it == m_message_received_events.end() ? nullptr : &it->second;
    }

    void NetPeer_Online::connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* info)
    {
        assert(s_callback_instance);
        assert(info);

        s_callback_instance->on_connection_status_changed(*info);
    }

    void NetPeer_Online::handle_message_internal(
        MsgID msg_id, EventHandle& event_handle,
        const INetPeer::ReceivedMessageLambda& event_lambda)
    {
        m_message_received_events[msg_id].add_listener(event_handle, event_lambda);
    }

    void NetPeer_Online::poll_incoming_messages()
    {
        while (true)
        {
            ISteamNetworkingMessage* msg = nullptr;
            const i32 num_msgs = poll_messages(&msg, 1);

            if (num_msgs == 0)
                break;

            if (num_msgs < 0)
            {
                log::error("Error checking for messages.");
                break;
            }

            const u32   msg_size  = msg->GetSize();
            const void* msg_data  = msg->GetData();
            if (msg_size < sizeof(MsgID))
            {
                log::warn("Received invalid package.");
                msg->Release();
                continue;
            }

            const MsgID msg_id = *static_cast<const MsgID*>(msg_data);
            const std::optional<PeerID> peer_id = handle_to_peer_id(msg->m_conn);
            if (!peer_id)
            {
                log::warn("Received message from invalid handle '{}'.", msg->m_conn);
                continue;
            }

            const auto event = get_msg_event(msg_id);
            if (!event)
            {
                log::warn("Received message with id '{}' not being listened for.", msg_id);
                msg->Release();
                continue;
            }

            const u8* msg_start = static_cast<const u8*>(msg_data) + sizeof(MsgID);
            const u8* msg_end   = msg_start + msg_size - sizeof(MsgID);
            const std::vector<u8> buffer { msg_start, msg_end };

            const ReceivedMessageEvent event_data { *peer_id, buffer };
            event->broadcast(event_data);

            msg->Release();
        }
    }

    void NetPeer_Online::poll_connection_state_changes()
    {
        s_callback_instance = this;
        m_interface->RunCallbacks();
    }
}