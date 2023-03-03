#include "rendering/pipeline.h"

#include "rendering/renderer.h"

#include <magic_enum.hpp>

namespace h2o::gfx
{
    PipelineCreateData::PipelineCreateData(IRenderer& renderer)
        : m_renderer(&renderer)
    {}

    PipelineCreateData& PipelineCreateData::add_shader(ShaderStage stage, const std::string& path)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:   m_vertex_shader   = { stage, path }; break;
        case ShaderStage::Fragment: m_fragment_shader = { stage, path }; break;
        case ShaderStage::Geometry: m_geometry_shader = { stage, path }; break;
        }

        return *this;
    }

    std::shared_ptr<IPipeline> PipelineCreateData::compile()
    {
        return m_renderer->compile_pipeline(*this);
    }
}