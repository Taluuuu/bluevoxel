#pragma once

#include "core/types.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
    inline Emitter& operator<<(Emitter& out, const v3i& v)
    {
        out << Flow;
        out << BeginSeq << v.x << v.y << v.z << EndSeq;
        return out;
    }

    template<>
    struct convert<v3i>
    {
        static bool decode(const Node& node, v3i& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<i32>();
            rhs.y = node[1].as<i32>();
            rhs.z = node[2].as<i32>();
            return true;
        }
    };

    inline Emitter& operator<<(Emitter& out, const v2 v)
    {
        out << Flow;
        out << BeginSeq << v.x << v.y << EndSeq;
        return out;
    }

    template<>
    struct convert<v2>
    {
        static bool decode(const Node& node, v2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<f32>();
            rhs.y = node[1].as<f32>();
            return true;
        }
    };
}