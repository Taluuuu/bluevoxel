#pragma once

#include "renderer_enums.h"

#include <string>

namespace engine
{
    class IPipeline;

    class PipelineFactory
    {
    public:

        PipelineFactory();

        PipelineFactory attach_shader(ShaderType type, const std::string& path);

    
    private:

        

    };
}