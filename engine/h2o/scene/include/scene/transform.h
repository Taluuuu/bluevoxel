#pragma once

#include "core/types.h"

namespace h2o
{
    struct Position
    {
        v3 position{ 0.0f };

        template<typename S>
        void serialize(S& s)
        { s(position); }
    };

    struct LocalPosition
    {
        v3 val{};

        template<typename S>
        void serialize(S& s)
        { s(val); }
    };

    struct Rotation
    {
        v3 rotation{ 0.0f };

        template<typename S>
        void serialize(S& s)
        { s(rotation); }
    };

    struct LocalRotation
    {
        v3 val{};

        template<typename S>
        void serialize(S& s)
        { s(val); }
    };

    struct Scale
    {
        v3 scale{ 1.0f };

        template<typename S>
        void serialize(S& s)
        { s(scale); }
    };

    struct LocalScale
    {
        v3 val{ 1.0f };

        template<typename S>
        void serialize(S& s)
        { s(val); }
    };

    struct Velocity
    {
        v3 velocity{ 0.0f };

        template<typename S>
        void serialize(S& s)
        { s(velocity); }
    };

    struct GlobalTransform
    {
        m4 mat{ 1.0f };

        // [[nodiscard]] v3 position() const { return ; }
    };
}