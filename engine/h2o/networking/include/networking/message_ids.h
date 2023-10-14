#pragma once

#include "networking_types.h"

namespace h2o::msg_ids
{
    const MsgID test = 100;

    // Scene replication
    const MsgID player_join = 200;
    const MsgID transform_update = 201;

    // Voxels
    const MsgID chunk_fetch_request = 300;
    const MsgID chunk_fetch_result = 301;
    const MsgID block_placed = 302;
}