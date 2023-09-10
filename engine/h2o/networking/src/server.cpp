#include "networking/server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/networking_module.h"

namespace h2o
{
    Server::~Server()
    {
        stop();
    }

    static ENetHost* create_server(u16 port)
    {
        const size_t max_clients = 32;
        const size_t channel_count = 2;

        const ENetAddress address { ENET_HOST_ANY, port };

        if (auto host = enet_host_create(&address, max_clients, channel_count, 0, 0))
            return host;

        return nullptr;
    }

    bool Server::start(u16 port)
    {
        if (m_server)
            return false;

        if (m_server = create_server(port); !m_server)
        {
            log::error("Failed to create ENet server.");
            return false;
        }

        log::info("Started ENet server on port {}.", port);

        auto networking_module = g_engine->get_module<NetworkingModule>();
        assert(networking_module);
        networking_module->register_server(*this);

        set_tick_phases(TickPhase::Update);

        return true;
    }

    void Server::stop(bool unregister_from_module)
    {
        if (!m_server)
            return;

        enet_host_destroy(m_server);
        m_server = nullptr;

        if (unregister_from_module)
        {
            auto networking_module = g_engine->get_module<NetworkingModule>();
            assert(networking_module);

            networking_module->unregister_server(*this);
        }
    }

    void Server::update(f32 delta_time)
    {
        ENetEvent event;
        while (enet_host_service(m_server, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                log::info("New client connected");

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }
}