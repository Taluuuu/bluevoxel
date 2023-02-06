#include "core/module.h"

#include <cassert>

namespace engine
{
    Module::Module(Engine& engine)
        : m_engine(&engine) {}

    bool Module::init(const GameInfo& game_info)
    {
        assert(!m_is_initialized);
        m_is_initialized = true;
        return true;
    }

    void Module::cleanup()
    {
        m_is_initialized = false;
    }
}