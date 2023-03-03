#include "core/module.h"

#include <cassert>

namespace h2o
{
    Module::Module(Engine& engine)
        : m_engine(&engine) {}
}