#pragma once

#include <ranges>

#include "networking_types.h"

#include <steam/steamnetworkingtypes.h>

namespace h2o
{
    class PeerIDManager
    {
    public:

        std::optional<PeerID> add_peer(HSteamNetConnection handle);
        void remove_peer(HSteamNetConnection handle);
        void clear();

        [[nodiscard]] std::span<const PeerID> peers() const { return m_peer_ids; }
        [[nodiscard]] std::optional<PeerID> get_peer_id(HSteamNetConnection handle) const;
        [[nodiscard]] std::optional<HSteamNetConnection> get_handle(PeerID peer_id) const;

    private:

        std::unordered_map<PeerID, HSteamNetConnection> m_peer_id_to_handle{};
        std::unordered_map<HSteamNetConnection, PeerID> m_handle_to_peer_id{};
        std::vector<PeerID> m_peer_ids{};

        PeerID m_current_peer_id = 1000;

    };
}
