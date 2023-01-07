#pragma once

#include "rendering/renderer_enums.h"

namespace engine
{
    // Charlo de demain
    // Le renderer contain des pipelines par nom
    // 
    class IPipeline
    {
    public:

        ~IPipeline() {}

        virtual IPipeline* add_shader(engine::ShaderType type, const std::string& path) = 0;

        virtual IPipeline* compile() = 0;

    };
}