#pragma once

#include "types.h"

#include <magic_enum.hpp>

namespace h2o
{
    namespace TickPhase
    {
        enum Type : u32
        {
            None = 0,
            FrameStart = 1 << 0,
            Update = 1 << 1,
            NetworkUpdate = 1 << 2,
            PreRender = 1 << 3,
            Render = 1 << 4,
            PostRender = 1 << 5,
            FrameEnd = 1 << 6,
        };
    }

    inline constexpr TickPhase::Type operator<<(TickPhase::Type phase, int shift)
    {
        using type = std::underlying_type_t<TickPhase::Type>;
        return static_cast<TickPhase::Type>(static_cast<type>(phase) << shift);
    }

    inline TickPhase::Type operator|(TickPhase::Type lhs, TickPhase::Type rhs)
    {
        return static_cast<TickPhase::Type>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
    }

    constexpr size_t tick_phase_count = magic_enum::enum_count<TickPhase::Type>();

    class Tickable
    {
    public:

        Tickable() = default;
        Tickable(const Tickable&) = delete;
        Tickable(Tickable&&) = delete;
        virtual ~Tickable();

        virtual void frame_start(f32 delta_time)    { assert(false); }
        virtual void update(f32 delta_time)         { assert(false); }
        virtual void network_update(f32 delta_time) { assert(false); }
        virtual void pre_render(f32 delta_time)     { assert(false); }
        virtual void render(f32 delta_time)         { assert(false); }
        virtual void post_render(f32 delta_time)    { assert(false); }
        virtual void frame_end(f32 delta_time)      { assert(false); }

    protected:

        void set_tick_phases(TickPhase::Type tick_phases);

    private:

        TickPhase::Type m_tick_phases = TickPhase::None;

    };
}