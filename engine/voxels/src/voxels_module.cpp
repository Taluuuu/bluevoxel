#include "voxels/voxels_module.h"

namespace h2o::vxl
{
    VoxelsModule::VoxelsModule(Engine &engine)
        : Module(engine)
    {

    }

    std::string_view VoxelsModule::module_name() const
    {
        return "voxels";
    }

    std::vector<std::type_index> VoxelsModule::dependencies() const
    {
        return {};
    }
}