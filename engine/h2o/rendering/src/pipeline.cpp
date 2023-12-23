#include "rendering/pipeline.h"

#include "rendering/renderer_base.h"

#include <magic_enum.hpp>

namespace h2o::gfx
{
    PipelineCreateData::PipelineCreateData(Renderer_Base& renderer)
        : m_renderer(&renderer)
    {}

    PipelineCreateData& PipelineCreateData::add_shader(ShaderStage stage, const std::string& path)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:   m_vertex_shader   = ShaderCreateData{ path }; break;
        case ShaderStage::Fragment: m_fragment_shader = ShaderCreateData{ path }; break;
        case ShaderStage::Geometry: m_geometry_shader = ShaderCreateData{ path }; break;
        }

        return *this;
    }

    PipelineCreateData& PipelineCreateData::with_feature(PipelineFeature::Type pipeline_feature)
    {
        m_pipeline_config.pipeline_features = m_pipeline_config.pipeline_features | pipeline_feature;
        return *this;
    }

    PipelineCreateData& PipelineCreateData::with_blend_config(BlendEquation equation, BlendFactor source_factor,BlendFactor dest_factor)
    {
        m_pipeline_config.blend_equation = equation;
        m_pipeline_config.blend_source_factor = source_factor;
        m_pipeline_config.blend_dest_factor = dest_factor;
        return *this;
    }

    std::shared_ptr<IPipeline> PipelineCreateData::compile()
    {
        return m_renderer->compile_pipeline(*this);
    }
}