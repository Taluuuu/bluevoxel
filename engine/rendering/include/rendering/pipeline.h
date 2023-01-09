#pragma once

#include "rendering/renderer_enums.h"

namespace engine
{
    class IPipeline
    {
    public:

        ~IPipeline() {}

        virtual IPipeline& add_shader(engine::ShaderType type, const std::string& path) = 0;

        virtual IPipeline& compile() = 0;

        virtual bool is_ready() const = 0;

    };
}