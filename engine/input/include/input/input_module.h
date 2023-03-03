#pragma once

#include "core/module.h"

namespace h2o
{
    class InputModule
        : public Module
    {
        explicit InputModule(Engine& engine);
        ~InputModule() override = default;
    };
}