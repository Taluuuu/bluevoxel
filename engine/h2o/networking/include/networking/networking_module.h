#pragma once

#include "core/events.h"
#include "core/module.h"
#include "networking_types.h"

#include <set>
#include <unordered_map>

namespace h2o
{
    class NetPeer;

    struct PeerCreatedEvent { NetPeer& peer; };

    class NetworkingModule : public IModule
    {
    public:

        ~NetworkingModule() override = default;

        void register_peer(NetPeer& peer);
        void unregister_peer(NetPeer& peer);

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_networking_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        Event<PeerCreatedEvent> on_peer_created{};

    private:

        std::set<NetPeer*> m_active_peers{};

    };
}