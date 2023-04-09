#include "core/tickable.h"

#include "core/engine.h"

#include <magic_enum.hpp>

namespace h2o
{
    Tickable::~Tickable()
    {
        assert(g_engine);
        g_engine->unregister_tickable(*this, m_tick_phases);
    }

    void Tickable::set_tick_phases(TickPhase tick_phases)
    {
        assert(g_engine);
        g_engine->unregister_tickable(*this, m_tick_phases);
        g_engine->register_tickable(*this, tick_phases);
        m_tick_phases = tick_phases;
    }
}