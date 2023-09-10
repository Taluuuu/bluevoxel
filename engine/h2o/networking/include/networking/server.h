#pragma once

#include "core/tickable.h"

#include <enet/enet.h>

namespace h2o
{
    class Server : public Tickable
    {
    public:

        Server() = default;
        ~Server() override;

        bool start(u16 port);
        void stop(bool unregister_from_module = false);

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        ENetHost* m_server { nullptr };

    };
}