#include "input/input_module.h"

#include "windowing/windowing_module.h"

#include <magic_enum.hpp>

namespace h2o
{
    InputModule::InputModule(Engine& engine) : Module(engine)
    {
        m_key_states.resize(magic_enum::enum_count<Key>(), {});
    }

    std::vector<std::type_index> InputModule::dependencies() const
    {
        return { typeid(h2o::WindowingModule) };
    }
}