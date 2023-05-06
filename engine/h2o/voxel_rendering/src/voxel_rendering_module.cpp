#include "voxel_rendering/voxel_rendering_module.h"

#include "core/engine.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    std::vector<std::type_index> VoxelRenderingModule::dependencies() const
    {
        return {};
    }

    bool VoxelRenderingModule::init(Engine& engine)
    {
        m_voxel_module = engine.get_module<VoxelModule>();
        if (!m_voxel_module)
            return false;

        // TODO: Load voxel meshes and textures

        return true;
    }
}