#pragma once

#include "core/tickable.h"

#include <enet/enet.h>

namespace h2o
{
    class Client : public Tickable
    {
    public:

        Client() = default;
        ~Client() override;

        bool connect(const std::string& hostname, u16 port);
        void disconnect(bool unregister_from_module = true);

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        [[nodiscard]] ENetHost* create_host() const;
        [[nodiscard]] ENetPeer* create_peer(const std::string& hostname, u16 port) const;

    private:

        static constexpr size_t channel_count = 2;

        ENetHost* m_client { nullptr };
        ENetPeer* m_peer { nullptr };

    };
}