#pragma once

#include "networking/message_ids.h"
#include "transform.h"

namespace h2o::net_msg
{
    struct SpawnEntity
    {
        std::vector<u8> serialized_components{};

        template<typename S>
        void serialize(S& s)
        { s(serialized_components); }

        static constexpr MsgID message_id = msg_ids::spawn_entity;
    };

    // Update all components of a single type for all entities in the registry
    struct UpdateComponent
    {
        entt::id_type component_type{};
        std::vector<u32> entity_ids{};
        std::vector<u8> serialized_components{};

        template<typename S>
        void serialize(S& s)
        { s(component_type, entity_ids, serialized_components); }

        static constexpr MsgID message_id = msg_ids::update_component;
    };
}