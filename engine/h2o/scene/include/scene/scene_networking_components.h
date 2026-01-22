#pragma once

#include "networking/networking_types.h"

namespace h2o
{
    struct NetworkSync
    {
        PeerID owner{};
        u32 entity_id{};

        template<typename S>
        void serialize(S& s)
        { s(entity_id, owner); }
    };

    template<class T>
    struct Dirty {};
}
