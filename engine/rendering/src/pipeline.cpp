#include "rendering/pipeline.h"

#include "rendering/renderer.h"

#include <magic_enum.hpp>

namespace engine
{
    PipelineFactory::PipelineFactory(const std::shared_ptr<IRenderer>& renderer)
        : m_renderer(renderer)
    {
        m_shaders.resize(magic_enum::enum_count<ShaderStage>(), std::nullopt);
    }

    PipelineFactory& PipelineFactory::add_shader(ShaderStage stage, const std::string& path)
    {
        auto stage_index = magic_enum::enum_index<ShaderStage>(stage);
        if (stage_index.has_value())
            m_shaders[*stage_index] = { stage, path };

        return *this;
    }

    IPipeline* PipelineFactory::compile()
    {
        return m_renderer->compile_pipeline(*this);
    }
}