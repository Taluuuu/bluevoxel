#pragma once

#include "core/events.h"
#include "voxel/voxel_events.h"

namespace h2o
{
    class ChunkColumn;

    class IVoxelTransport
    {
    public:

        virtual ~IVoxelTransport() = default;

        // Shared functions and events
        // Block placing
        virtual void request_block_place(const voxel::BlockPlaceEvent& event) = 0;
        virtual Event<voxel::BlockPlaceEvent>& on_block_place() = 0;

        // Client functions and events
        // Chunk fetching
        virtual void fetch_chunk_columns(const voxel::ChunkColumnRequestedEvent& event) = 0;
        virtual Event<voxel::ReceivedChunkColumnEvent>& on_received_chunk_column() = 0;

        // Server functions and events
        // Chunk fetching
        virtual void send_chunk_column(const voxel::ReceivedChunkColumnEvent& event) = 0;
        virtual Event<voxel::ChunkColumnRequestedEvent>& on_chunk_column_requested() = 0;

    };
}