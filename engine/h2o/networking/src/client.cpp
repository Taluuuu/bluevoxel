#include "networking/client.h"

#include "core/engine.h"
#include "networking/networking_module.h"

namespace h2o
{
    Client::~Client()
    {
        disconnect();
    }

    bool Client::connect(const std::string& hostname, u16 port)
    {
        if (m_client)
            return false;

        if (m_client = create_host(); !m_client)
        {
            log::error("Failed to create ENet client.");
            return false;
        }

        if (m_peer = create_peer(hostname, port); !m_peer)
        {
            log::error("Failed to create ENet peer.");
            return false;
        }

        ENetEvent event;
        if (enet_host_service(m_client, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            log::info("Connected ENet client to {}:{}.", hostname, port);
        }
        else
        {
            enet_peer_reset(m_peer);
            m_peer = nullptr;

            log::error("Failed to connect ENet client to {}:{}.", hostname, port);

            return false;
        }

        auto networking_module = g_engine->get_module<NetworkingModule>();
        assert(networking_module);
        networking_module->register_client(*this);

        set_tick_phases(TickPhase::Update);

        return true;
    }

    void Client::disconnect(bool unregister_from_module)
    {
        if (!m_client)
            return;

        enet_host_destroy(m_client);
        m_client = nullptr;

        if (unregister_from_module)
        {
            auto networking_module = g_engine->get_module<NetworkingModule>();
            assert(networking_module);

            networking_module->unregister_client(*this);
        }
    }

    void Client::update(f32 delta_time)
    {
        ENetEvent event;
        while (enet_host_service(m_client, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
//                const auto& addr = event.peer->address.host.u;
//                const std::string address_str = fmt::format(
//                    "{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}",
//                    addr.Word[0], addr.Word[1], addr.Word[2], addr.Word[3],
//                    addr.Word[4], addr.Word[5], addr.Word[6], addr.Word[7]);
//
//                log::info("New client connected from {:x}:{}",
//                    address_str, event.peer->address.port);

                log::info("Connected to server !");

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
                log::info("Disconnected from server !");
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }

    ENetHost* Client::create_host() const
    {
        if (auto host = enet_host_create(nullptr, 1, channel_count, 0, 0))
            return host;

        return nullptr;
    }

    ENetPeer* Client::create_peer(const std::string& hostname, u16 port) const
    {
        assert(m_client);

        ENetAddress address;
        enet_address_set_host(&address, hostname.c_str());
        address.port = port;

        if (auto peer = enet_host_connect(m_client, &address, channel_count, 0))
            return peer;

        return nullptr;
    }
}