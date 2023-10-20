#pragma once

#include "voxel/chunk_manager_base.h"

namespace h2o
{
    class Client;

    class ChunkManager_Client : public ChunkManager_Base
    {
    public:

        explicit ChunkManager_Client(Client& client);
        ~ChunkManager_Client() override = default;

        // IBlockContainer interface
        bool set_block_at(const v3i& block_pos, Block block) override;

    private:

        Client* m_client = nullptr;

    };
}