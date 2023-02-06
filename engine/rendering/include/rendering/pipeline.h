#pragma once

#include "rendering/renderer_enums.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace engine
{
    class IRenderer;

    class IPipeline
    {
    public:

        virtual ~IPipeline() {}

    };

    class PipelineFactory
    {
    public:

        PipelineFactory(const std::shared_ptr<IRenderer>& renderer);

        PipelineFactory& add_shader(ShaderStage stage, const std::string& path);

        IPipeline* compile();

    public:

        struct ShaderCreateData
        {
            ShaderStage stage;
            std::string path;
        };

    public:

        auto shader_create_params() const { return m_shaders; }

    private:

        std::vector< std::optional<ShaderCreateData> > m_shaders;

        std::shared_ptr<IRenderer> const m_renderer = nullptr;

    };
}