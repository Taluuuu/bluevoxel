#include "voxels/voxels_module.h"

namespace engine
{
    VoxelsModule::VoxelsModule(Engine &engine)
        : Module(engine)
    {

    }

    std::string_view VoxelsModule::get_module_name() const
    {
        return "voxels";
    }

    std::vector<std::type_index> VoxelsModule::get_dependencies() const
    {
        return {};
    }
}