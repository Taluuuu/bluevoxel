#pragma once

#include "rendering/renderer_enums.h"

#include <string>

namespace engine
{
    class IPipeline
    {
    public:

        ~IPipeline() {}

        virtual IPipeline& add_shader(ShaderStage stage, const std::string& path) = 0;

        virtual IPipeline& compile() = 0;

        virtual bool is_ready() const = 0;

    };
}