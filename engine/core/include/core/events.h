#pragma once

#include "types.h"

#include <functional>
#include <vector>

namespace h2o
{
    class EventHandle
    {
    public:

        EventHandle() = default;
        explicit EventHandle(i32 id) : m_id(id) {}

        [[nodiscard]] bool is_valid() const { return m_id >= 0; }

        bool operator==(const EventHandle& rhs) const { return m_id == rhs.m_id; }

    private:

        i32 m_id = -1;

    };

    // To add a very needed unbind method on an EventHandle that would be called
    // either manually or during destruction, I could create a base Event class
    // the templated version would inherit from (BaseEvent). This would contain
    // all logic that does not need the template. Removing listeners does not
    // require the template type.
    //
    // EDIT: I can't create the Listener struct without the template type. Need
    // to think about this some more.

    template<class T>
    class Event
    {
    public:

        Event() = default;
        Event(const Event&) = delete;
        Event(Event&&) = delete;
        ~Event() { assert(m_listeners.empty()); }

        void broadcast(const T& event) const
        {
            for (const auto& listener : m_listeners)
            {
                listener.callback(event);
            }
        }

        [[nodiscard]] EventHandle add_listener(const std::function<void(const T&)>& callback)
        {
            EventHandle new_handle(s_next_handle_id++);
            m_listeners.push_back({ callback, new_handle });
            return new_handle;
        }

        void remove_listener(const EventHandle& handle)
        {
            if (!handle.is_valid())
                return;

            m_listeners.erase(std::remove_if(
                m_listeners.begin(), 
                m_listeners.end(), 
                [&handle](const auto& listener)
                {
                    return listener.handle == handle;
                }));
        }

    private:

        struct Listener
        {
            std::function<void(const T&)> callback;
            EventHandle handle;
        };

        std::vector<Listener> m_listeners;

        // Note: not thread safe
        static i32 s_next_handle_id;

    };

    template<class T>
    i32 Event<T>::s_next_handle_id = 0;
}