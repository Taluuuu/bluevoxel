#pragma once

#include <cassert>
#include <mutex>
#include <queue>

namespace h2o
{
    template<class PriorityType, class ValueType>
    class ThreadSafePriorityQueue
    {
    public:

        void push(const PriorityType& priority, const ValueType& value)
        {
            std::lock_guard lock { m_mutex };
            m_queue.emplace({ priority, value });
        }

        ValueType pop()
        {
            std::lock_guard lock { m_mutex };

            assert(!is_empty());
            m_queue.pop();
        }

        const ValueType& top() const
        {
            std::lock_guard lock { m_mutex };

            assert(!is_empty());
            return m_queue.top();
        }

        [[nodiscard]] bool is_empty() const
        {
            std::lock_guard lock { m_mutex };
            return m_queue.empty();
        }

    private:

        struct QueueElem
        {
            PriorityType priority{};
            ValueType value{};

            bool operator<(const QueueElem& other)
            { return priority < other.priority; }
        };

        std::mutex m_mutex{};
        std::priority_queue<QueueElem> m_queue{};

    };
}