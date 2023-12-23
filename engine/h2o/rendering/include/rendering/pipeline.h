#pragma once

#include "core/types.h"
#include "rendering/renderer_enums.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace h2o::gfx
{
    class Renderer_Base;

    struct PipelineConfig
    {
        PipelineFeature::Type pipeline_features = PipelineFeature::None;

        BlendEquation blend_equation = BlendEquation::Add;
        BlendFactor blend_source_factor = BlendFactor::SrcAlpha;
        BlendFactor blend_dest_factor = BlendFactor::SrcAlpha;
    };

    class IPipeline
    {
    public:

        virtual ~IPipeline() = default;

        virtual void set_uniform_mat4(i32 location, const m4& value) const = 0;
        virtual void set_uniform_vec3(i32 location, const v3& value) const = 0;
        virtual void set_uniform_ivec3(i32 location, const v3i& value) const = 0;
        virtual void set_uniform_float(i32 location, f32 value) const = 0;
        virtual void set_uniform_int(i32 location, i32 value) const = 0;

        [[nodiscard]] virtual i32 get_uniform_location(const std::string& name) const = 0;
        [[nodiscard]] virtual i32 get_attribute_location(const std::string& name) const = 0;

        [[nodiscard]] virtual const PipelineConfig& pipeline_config() const = 0;

    };

    class PipelineCreateData
    {
    public:

        explicit PipelineCreateData(Renderer_Base& renderer);

        PipelineCreateData& add_shader(ShaderStage stage, const std::string& path);
        PipelineCreateData& with_feature(PipelineFeature::Type pipeline_feature);
        PipelineCreateData& with_blend_config(BlendEquation equation, BlendFactor source_factor, BlendFactor dest_factor);

        std::shared_ptr<IPipeline> compile();

        struct ShaderCreateData
        {
            std::string path;
        };

        [[nodiscard]] const std::optional<ShaderCreateData>& vertex_shader()   const { return m_vertex_shader;   }
        [[nodiscard]] const std::optional<ShaderCreateData>& fragment_shader() const { return m_fragment_shader; }
        [[nodiscard]] const std::optional<ShaderCreateData>& geometry_shader() const { return m_geometry_shader; }

        [[nodiscard]] PipelineConfig pipeline_config() const { return m_pipeline_config; }

    private:

        std::optional<ShaderCreateData> m_vertex_shader{};
        std::optional<ShaderCreateData> m_fragment_shader{};
        std::optional<ShaderCreateData> m_geometry_shader{};

        PipelineConfig m_pipeline_config{};

        Renderer_Base* const m_renderer = nullptr;

    };
}