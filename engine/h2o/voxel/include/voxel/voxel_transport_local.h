#pragma once

#include "voxel_transport.h"

namespace h2o
{
    class VoxelTransport_Local : public IVoxelTransport
    {
    public:

        ~VoxelTransport_Local() override = default;

        // IVoxelTransport interface begin
        void request_block_place(const voxel::BlockPlaceEvent& event) override;
        Event<voxel::BlockPlaceEvent>& on_block_place() override { return m_block_place_event; }
        void fetch_chunk_columns(const voxel::ChunkColumnRequestedEvent& event) override;
        Event<voxel::ReceivedChunkColumnEvent>& on_received_chunk_column() override { return m_received_chunk_column_event; }
        void send_chunk_column(const voxel::ReceivedChunkColumnEvent& event) override;
        Event<voxel::ChunkColumnRequestedEvent>& on_chunk_column_requested() override { return m_chunk_column_requested_event; }
        // IVoxelTransport interface end

    private:

        Event<voxel::ReceivedChunkColumnEvent> m_received_chunk_column_event{};
        Event<voxel::ChunkColumnRequestedEvent> m_chunk_column_requested_event{};
        Event<voxel::BlockPlaceEvent> m_block_place_event{};

    };
}