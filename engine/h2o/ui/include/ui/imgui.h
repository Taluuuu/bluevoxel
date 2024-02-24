#pragma once

#include "core/types.h"

#define IM_VEC2_CLASS_EXTRA                                                 \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {}                   \
        operator v2() const { return v2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
        constexpr ImVec4(const v4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}   \
        operator v4() const { return v4(x,y,z,w); }

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>