#pragma once

#include "networking/networking_types.h"

namespace h2o
{
    using SerializeFn   = void(*)(entt::registry&, entt::entity, net_utils::Writer&);
    using DeserializeFn = void(*)(entt::registry&, entt::entity, net_utils::Reader&);

    struct ReplicatedComponentDesc
    {
        entt::id_type type;
        SerializeFn serialize;
        DeserializeFn deserialize;
    };

    struct NetworkSync
    {
        PeerID owner{};

        template<typename S>
        void serialize(S& s)
        { s(owner); }
    };
}
