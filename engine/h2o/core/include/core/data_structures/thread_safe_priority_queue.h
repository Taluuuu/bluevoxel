#pragma once

#include <mutex>
#include <optional>
#include <queue>

namespace h2o
{
    template<class PriorityType, class ValueType>
    class ThreadSafePriorityQueue
    {
    public:

        ThreadSafePriorityQueue() = default;
//        ThreadSafePriorityQueue(const ThreadSafePriorityQueue& other) = delete;
//        {
//            std::lock_guard lock { other.m_mutex };
//            m_queue = other.m_queue;
//        }

        ThreadSafePriorityQueue(ThreadSafePriorityQueue<PriorityType, ValueType>&& other)
        {
            std::lock_guard lock { other.m_mutex };
            m_queue = std::move(other.m_queue);
        }

//        ThreadSafePriorityQueue& operator=(ThreadSafePriorityQueue other)
//        {
//            std::lock_guard lock { other.m_mutex };
//            std::swap(m_queue, other.m_queue);
//            return *this;
//        }

        void push(const PriorityType& priority, const ValueType& value)
        {
            std::lock_guard lock { m_mutex };
            m_queue.emplace({ priority, value });
        }

        std::optional<ValueType> pop()
        {
            std::lock_guard lock { m_mutex };

            if (m_queue.empty())
                return {};

            ValueType val = m_queue.top();
            m_queue.pop();

            return val;
        }

    private:

        struct QueueElem
        {
            PriorityType priority{};
            ValueType value{};

            bool operator<(const QueueElem& other)
            { return priority < other.priority; }
        };

        mutable std::mutex m_mutex{};
        std::priority_queue<QueueElem> m_queue{};

    };
}