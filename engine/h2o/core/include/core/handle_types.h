#pragma once

#include <oup/observable_unique_ptr.hpp>

namespace h2o
{
    template<typename T>
    using OwningHandle = oup::observable_unique_ptr<T>;

    template<typename T>
    using WeakHandle = oup::observer_ptr<T>;
}