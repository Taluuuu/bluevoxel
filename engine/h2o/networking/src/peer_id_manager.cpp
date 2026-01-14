#include "networking/peer_id_manager.h"

#include "core/log.h"

namespace h2o
{
    std::optional<PeerID> PeerIDManager::add_peer(const HSteamNetConnection handle)
    {
        if (m_handle_to_peer_id.contains(handle))
        {
            log::error("Tried to generate id for new peer, but handle is already assigned.");
            return 0;
        }

        const PeerID peer_id = m_current_peer_id++;
        m_handle_to_peer_id[handle] = peer_id;
        m_peer_id_to_handle[peer_id] = handle;
        m_peer_ids.push_back(peer_id);
        return peer_id;
    }

    void PeerIDManager::remove_peer(const HSteamNetConnection handle)
    {
        if (const auto peer_id = get_peer_id(handle))
        {
            m_peer_id_to_handle.erase(*peer_id);
            erase(m_peer_ids, *peer_id);
        }

        m_handle_to_peer_id.erase(handle);
    }

    void PeerIDManager::clear()
    {
        m_peer_id_to_handle.clear();
        m_handle_to_peer_id.clear();
        m_peer_ids.clear();
    }

    std::optional<PeerID> PeerIDManager::get_peer_id(const HSteamNetConnection handle) const
    {
        const auto it = m_handle_to_peer_id.find(handle);
        return it == m_handle_to_peer_id.end() ? std::optional<PeerID>() : it->second;
    }

    std::optional<HSteamNetConnection> PeerIDManager::get_handle(const PeerID peer_id) const
    {
        const auto it = m_peer_id_to_handle.find(peer_id);
        return it == m_peer_id_to_handle.end() ? std::optional<HSteamNetConnection>() : it->second;
    }
}
