#include "networking/net_peer_local.h"

#include "core/log.h"

namespace h2o
{
    void NetPeer_Local::send_message_internal(PeerID client_id, const void* data, size_t size)
    {
        if (!std::ranges::contains(peers(), client_id))
        {
            log::warn("Invalid client id {} for local net peer.", client_id);
            return;
        }

        const MsgID msg_id = *static_cast<const MsgID*>(data);

        // The message is 'received' here...
        // NOTE: This is very inefficient as the message is needlessly encoded then decoded,
        // but it will work for now.
        if (const auto it = m_message_received_events.find(msg_id);
            it != m_message_received_events.end())
        {
            const u8* msg_start = static_cast<const u8*>(data) + sizeof(MsgID);
            const u8* msg_end   = msg_start + size - sizeof(MsgID);
            const std::vector<u8> buffer { msg_start, msg_end };

            const ReceivedMessageEvent event_data { 0, buffer };
            it->second.broadcast(event_data);

        }
        else
        {
            log::warn("Received message not being handled with id {}", msg_id);
        }
    }

    void NetPeer_Local::handle_message_internal(
        MsgID msg_id, EventHandle& event_handle,
        const INetPeer::ReceivedMessageLambda& event_lambda)
    {
        m_message_received_events[msg_id].add_listener(event_handle, event_lambda);
    }

    std::span<const PeerID> NetPeer_Local::peers() const
    {
        static const std::vector<PeerID> local_peers{ 0 };
        return local_peers;
    }

    u32 NetPeer_Local::local_peer_id() const
    {
        return 0;
    }
}
