#pragma once

#include "types.h"

#include <magic_enum.hpp>
#include <queue>
#include <set>

namespace h2o
{
    namespace TickPhase
    {
        enum Type : u32
        {
            None          = 0,
            FrameStart    = 1 << 0,
            Update        = 1 << 1,
            PostUpdate    = 1 << 2,
            NetworkUpdate = 1 << 3,
            PreRender     = 1 << 4,
            Render        = 1 << 5,
            PostRender    = 1 << 6,
            FrameEnd      = 1 << 7,
        };

        inline TickPhase::Type operator|(TickPhase::Type lhs, TickPhase::Type rhs)
        {
            return static_cast<TickPhase::Type>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
        }
    }

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
        void run_post_update(f32 delta_time);
        void run_network_update(f32 delta_time);
        void run_pre_render();
        void run_render();
        void run_post_render();
        void run_frame_end(f32 delta_time);

        virtual void frame_start(f32 delta_time)    { assert(false); }
        virtual void update(f32 delta_time)         { assert(false); }
        virtual void post_update(f32 delta_time)    { assert(false); }
        virtual void network_update(f32 delta_time) { assert(false); }
        virtual void pre_render()                   { assert(false); }
        virtual void render()                       { assert(false); }
        virtual void post_render()                  { assert(false); }
        virtual void frame_end(f32 delta_time)      { assert(false); }

    private:

        std::set<Tickable*> m_children{};
        std::set<Tickable*> m_pending_children{}; // Add children on the next frame
        Tickable* const m_owner = nullptr;

        TickPhase::Type m_tick_phases = TickPhase::None;
        bool m_enabled = true;

    };
}