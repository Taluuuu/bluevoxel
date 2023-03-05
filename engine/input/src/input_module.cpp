#include "input/input_module.h"

#include "windowing/windowing_module.h"

namespace h2o
{
    InputModule::InputModule(Engine& engine) : Module(engine)
    {

    }

    std::vector<std::type_index> InputModule::dependencies() const
    {
        return { typeid(h2o::WindowingModule) };
    }
}