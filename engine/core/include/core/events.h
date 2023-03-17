#pragma once

#include "types.h"

#include <functional>
#include <vector>

#define ID_NONE (-1)

namespace h2o
{
    class EventHandle;

    class EventBase
    {
    public:

        virtual void remove_listener(const EventHandle& event_handle) = 0;

    };

    class EventHandle
    {
    public:

        EventHandle() = default;
        ~EventHandle() { reset(); }
        EventHandle(const EventHandle&) = delete;
        EventHandle(EventHandle&&) = delete;

        bool operator==(const EventHandle& rhs) const { return m_id == rhs.m_id; }

        /**
         * Unbind the listener associated with this handle. Does nothing if this handle
         * is unused.
         * This function is called by the destructor.
         */
        void reset() { reset(nullptr, ID_NONE); }

    private:

        template<class T>
        friend class Event;

        void reset(EventBase* event, i32 id)
        {
            if (m_event)
            {
                assert(m_id != ID_NONE);

                m_event->remove_listener(*this);
                clear();
            }

            if (event)
            {
                assert(id != ID_NONE);

                m_event = event;
                m_id = id;
            }
        }

        [[nodiscard]] i32 id() const
        {
            return m_id;
        }

        void clear()
        {
            m_event = nullptr;
            m_id = ID_NONE;
        }

    private:

        EventBase* m_event = nullptr;
        i32 m_id = ID_NONE;

    };

    template<class T>
    class Event : public EventBase
    {
    public:

        Event() = default;
        Event(const Event&) = delete;
        Event(Event&&) = delete;
        ~Event()
        {
            while (!m_listeners.empty())
                remove_listener_impl(m_listeners.front().handle_id);
        }

        void broadcast(const T& event) const
        {
            for (const auto& listener : m_listeners)
                listener.callback(event);
        }

        void add_listener(EventHandle& handle, const std::function<void(const T&)>& callback)
        {
            handle.reset(this, s_next_handle_id++);
            m_listeners.push_back({ callback, handle.id() });
        }

        void remove_listener(const EventHandle& handle) override
        {
            // Calls another function as removing listeners is something we want to do
            // from the destructor
            remove_listener_impl(handle.id());
        }

    private:

        void remove_listener_impl(i32 id)
        {
            if (id == ID_NONE)
                return;

            const auto num_erased = std::erase_if(m_listeners,
                [id](const auto& listener)
                {
                    return listener.handle_id == id;
                });

            assert(num_erased < 2);
        }

    private:

        struct Listener
        {
            std::function<void(const T&)> callback;
            i32 handle_id;
        };

        std::vector<Listener> m_listeners;

        static i32 s_next_handle_id;

    };

    template<class T>
    i32 Event<T>::s_next_handle_id = 0;
}