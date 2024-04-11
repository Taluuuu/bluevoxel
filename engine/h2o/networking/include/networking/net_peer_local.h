#pragma once

#include "core/tickable.h"
#include "net_peer.h"

namespace h2o
{
    class NetPeer_Local : public INetPeer
    {
    public:

        NetPeer_Local() = default;
        ~NetPeer_Local() override = default;

        // INetPeer interface
        [[nodiscard]] const std::unordered_set<PeerID>& peers() const override;
        [[nodiscard]] bool is_connected() const override { return true; }
    protected:
        void send_message_internal(PeerID client_id, const void* data, size_t size) override;
        void handle_message_internal(MsgID msg_id, EventHandle& event_handle, const ReceivedMessageLambda& event_lambda) override;

    private:

        std::unordered_map<MsgID, Event<ReceivedMessageEvent>> m_message_received_events{};

    };
}