#include "core/tickable.h"

#include "core/engine.h"

#include <magic_enum.hpp>

namespace h2o
{
    Tickable::Tickable(Tickable* owner)
        : m_owner(owner)
    {
        if (m_owner)
            m_owner->add_child(*this);
    }

    Tickable::~Tickable()
    {
        if (m_owner)
            m_owner->remove_child(*this);
    }

    void Tickable::add_child(Tickable& tickable)
    {
        m_pending_children.emplace(&tickable);
    }

    void Tickable::remove_child(Tickable& tickable)
    {
        // NOTE: This could cause a problem if a child is removed on the same frame as it is added
        m_children.erase(&tickable);
    }

    void Tickable::run_frame_start(f32 delta_time)
    {
        while (!m_pending_children.empty())
        {
            m_children.emplace(m_pending_children.front());
            m_pending_children.pop();
        }

        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::FrameStart)
                frame_start(delta_time);

            for (Tickable* child: m_children)
                child->run_frame_start(delta_time);
        }
    }

    void Tickable::run_update(f32 delta_time)
    {
        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::Update)
                update(delta_time);

            for (Tickable* child: m_children)
                child->run_update(delta_time);
        }
    }

    void Tickable::run_network_update(f32 delta_time)
    {
        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::NetworkUpdate)
                network_update(delta_time);

            for (Tickable* child: m_children)
                child->run_network_update(delta_time);
        }
    }

    void Tickable::run_pre_render()
    {
        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::PreRender)
                pre_render();

            for (Tickable* child: m_children)
                child->run_pre_render();
        }
    }

    void Tickable::run_render()
    {
        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::Render)
                render();

            for (Tickable* child: m_children)
                child->run_render();
        }
    }

    void Tickable::run_post_render()
    {
        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::PostRender)
                post_render();

            for (Tickable* child: m_children)
                child->run_post_render();
        }
    }

    void Tickable::run_frame_end(f32 delta_time)
    {
        if (is_enabled())
        {
            if (m_tick_phases & TickPhase::FrameEnd)
                frame_end(delta_time);

            for (Tickable* child: m_children)
                child->run_frame_end(delta_time);
        }
    }
}