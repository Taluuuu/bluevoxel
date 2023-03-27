#pragma once

#include "core/types.h"

namespace h2o
{
    struct KeyState
    {
        u8 held               : 1 = false;
        u8 pressed_this_frame : 1 = false;
    };
}