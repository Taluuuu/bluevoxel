#include "voxel_server/voxel_server_module.h"

#include "voxel/voxel_module.h"
#include "voxel/chunk_generators/chunk_generator_flat.h"
#include "networking/networking_module.h"

namespace h2o
{
    bool VoxelServerModule::init(Engine& engine)
    {
        // Setup flat world gen
        auto flat_generator = std::make_unique<ChunkGenerator_Flat>();
        flat_generator->block_layers = { 3, 3, 3, 3, 3, 2, 2, 2, 1 };
        m_chunk_server.set_chunk_generator(std::move(flat_generator));

        m_chunk_server.start();

        return true;
    }

    void VoxelServerModule::cleanup()
    {
        m_chunk_server.stop();
    }

    std::string_view VoxelServerModule::module_name() const
    {
        return "h2o_voxel_server";
    }

    std::vector<std::type_index> VoxelServerModule::dependencies() const
    {
        return { typeid(NetworkingModule), typeid(VoxelModule) };
    }
}