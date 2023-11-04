#pragma once

#include "types.h"

#include <magic_enum.hpp>
#include <set>

namespace h2o
{
    namespace TickPhase
    {
        enum Type : u32
        {
            FrameStart = 1 << 0,
            Update = 1 << 1,
            NetworkUpdate = 1 << 2,
            PreRender = 1 << 3,
            Render = 1 << 4,
            PostRender = 1 << 5,
            FrameEnd = 1 << 6,
        };

        inline constexpr Type None = static_cast<Type>(0);
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

        explicit Tickable(Tickable* owner);
        Tickable(const Tickable&) = delete;
        Tickable(Tickable&&) = delete;
        virtual ~Tickable();

        void add_child(Tickable& tickable);
        void remove_child(Tickable& tickable);

        [[nodiscard]] bool is_enabled() const { return m_enabled; }
        void set_enabled(bool enabled) { m_enabled = enabled; }
        void enable()  { set_enabled(true);  }
        void disable() { set_enabled(false); }

    protected:

        void set_tick_phases(TickPhase::Type tick_phases) { m_tick_phases = tick_phases; }

        void run_frame_start(f32 delta_time);
        void run_update(f32 delta_time);
        void run_network_update(f32 delta_time);
        void run_pre_render();
        void run_render();
        void run_post_render();
        void run_frame_end(f32 delta_time);

        virtual void frame_start(f32 delta_time)    { assert(false); }
        virtual void update(f32 delta_time)         { assert(false); }
        virtual void network_update(f32 delta_time) { assert(false); }
        virtual void pre_render()                   { assert(false); }
        virtual void render()                       { assert(false); }
        virtual void post_render()                  { assert(false); }
        virtual void frame_end(f32 delta_time)      { assert(false); }

    private:

        std::set<Tickable*> m_children{};
        Tickable* const m_owner = nullptr;

        TickPhase::Type m_tick_phases = TickPhase::None;
        bool m_enabled = true;

    };
}