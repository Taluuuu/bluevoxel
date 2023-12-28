#pragma once

#include "core/module.h"

namespace h2o
{
    class AudioModule
        : public IModule
    {
    public:
        AudioModule() = default;
        ~AudioModule();
    };
}