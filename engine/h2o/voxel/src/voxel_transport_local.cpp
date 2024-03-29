#include "voxel/voxel_transport_local.h"

namespace h2o
{
    void VoxelTransport_Local::fetch_chunk_columns(const std::vector<v2i>& chunk_column_positions)
    {
        m_chunk_column_requested_event.broadcast({ 0, chunk_column_pos });
    }

    void VoxelTransport_Local::send_chunk_column(u32 client_id, const std::shared_ptr<ChunkColumn>& chunk_column)
    {
        m_received_chunk_column_event.broadcast({ chunk_column });
    }

    void VoxelTransport_Local::request_block_place(v3i block_pos, Block block)
    {
        m_block_place_event.broadcast({ block_pos, block });
    }
}