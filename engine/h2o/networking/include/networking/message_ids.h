#pragma once

#include "networking_types.h"

namespace h2o::msg_ids
{
    constexpr MsgID welcome = 99;
    constexpr MsgID test = 100;

    // Scene replication
    constexpr MsgID player_join = 200;
    constexpr MsgID transform_update = 201;
    constexpr MsgID actor_destroyed = 202;

    // Voxels
    constexpr MsgID chunk_fetch_request = 300;
    constexpr MsgID chunk_fetch_result = 301;
    constexpr MsgID block_placed = 302;

    // Weather
    constexpr MsgID time_changed = 400;
}