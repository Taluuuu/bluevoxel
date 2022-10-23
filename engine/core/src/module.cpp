#include "core/module.h"

#include <cassert>

namespace engine
{
    bool Module::init()
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