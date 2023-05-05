#pragma once

#include "types.h"

#include <magic_enum.hpp>

namespace h2o
{
    enum TickPhase : u32
    {
        FrameStart = 1 << 0,
        Update     = 1 << 1,
        PreRender  = 1 << 2,
        Render     = 1 << 3,
        PostRender = 1 << 4,
        FrameEnd   = 1 << 5,
    };

    inline constexpr TickPhase operator<<(TickPhase phase, int shift)
    {
        using type = std::underlying_type_t<TickPhase>;
        return static_cast<TickPhase>(static_cast<type>(phase) << shift);
    }

    inline TickPhase operator|(TickPhase lhs, TickPhase rhs)
    {
        return static_cast<TickPhase>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
    }

    constexpr size_t tick_phase_count = magic_enum::enum_count<TickPhase>();

    class Tickable
    {
    public:

        Tickable() = default;
        Tickable(const Tickable&) = delete;
        Tickable(Tickable&&) = delete;
        virtual ~Tickable();

        virtual void frame_start(f32 delta_time) { assert(false); }
        virtual void update(f32 delta_time)      { assert(false); }
        virtual void pre_render(f32 delta_time)  { assert(false); }
        virtual void render(f32 delta_time)      { assert(false); }
        virtual void post_render(f32 delta_time) { assert(false); }
        virtual void frame_end(f32 delta_time)   { assert(false); }

    protected:

        void set_tick_phases(TickPhase tick_phases);

    private:

        TickPhase m_tick_phases = static_cast<TickPhase>(0);

    };
}