#pragma once

#include "types.h"

namespace h2o
{
    class IInputModule
    {
    public:

        virtual ~IInputModule() = default;

        virtual void prepare() = 0;

    };

    class ITimeProvider
    {
    public:

        virtual ~ITimeProvider() = default;

        [[nodiscard]] virtual f64 time() const = 0;
        [[nodiscard]] virtual f64 delta_time() const = 0;

    };
}