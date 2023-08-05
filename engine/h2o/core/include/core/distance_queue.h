#pragma once

#include "core/types.h"

#include <functional>
#include <optional>
#include <vector>

namespace h2o
{
    template<class T>
    struct DistanceQueueElem
    {
        T value;
        f32 distance { 0.0f };

        [[nodiscard]] bool operator<(const DistanceQueueElem& other) const
        { return distance < other.distance; }
    };

    template<class T>
    using DistanceQueue = std::vector< DistanceQueueElem<T> >;

    namespace util
    {
        template<class T>
        bool distance_queue_contains(const DistanceQueue<T>& queue, const T& value)
        {
            auto value_it = std::find_if(queue.begin(), queue.end(),
                [&value](const DistanceQueueElem<T>& item) -> bool
                {
                    return item.value == value;
                }
            );

            return value_it != queue.end();
        }

        template<class T>
        void distance_queue_insert(DistanceQueue<T>& queue, const DistanceQueueElem<T>& elem)
        {
            for (i32 i = 0; i < queue.size(); i++)
            {
                if (elem.distance < queue[i].distance)
                {
                    queue.insert(queue.cbegin() + i, elem);
                    return;
                }
            }

            queue.push_back(elem);
        }

        // Pop if predicate is evaluated to true
        template<class T>
        std::optional< DistanceQueueElem<T> > distance_queue_pop(
            DistanceQueue<T>& queue,
            std::function<bool(const DistanceQueueElem<T>&)> predicate)
        {
            for (i32 i = 0; i < queue.size(); i++)
            {
                auto value = queue[i];
                if (predicate(value))
                {
                    queue.erase(queue.cbegin() + i);
                    return value;
                }
            }

            return std::nullopt;
        }
    }
}