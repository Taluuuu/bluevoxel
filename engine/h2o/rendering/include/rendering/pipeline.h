#pragma once

#include "core/types.h"
#include "rendering/renderer_enums.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace h2o::gfx
{
    class IRenderer;

    class IPipeline
    {
    public:

        virtual ~IPipeline() = default;

        virtual void set_uniform_mat4(i32 location, const m4& value) const = 0;

        virtual void set_uniform_ivec3(i32 location, const v3i& value) const = 0;

        virtual void set_uniform_int(i32 location, i32 value) const = 0;

    };

    class PipelineCreateData
    {
    public:

        explicit PipelineCreateData(IRenderer& renderer);

        PipelineCreateData& add_shader(ShaderStage stage, const std::string& path);

        std::shared_ptr<IPipeline> compile();

        struct ShaderCreateData
        {
            ShaderStage stage;
            std::string path;
        };

        [[nodiscard]] const std::optional<ShaderCreateData>& vertex_shader()   const { return m_vertex_shader;   }
        [[nodiscard]] const std::optional<ShaderCreateData>& fragment_shader() const { return m_fragment_shader; }
        [[nodiscard]] const std::optional<ShaderCreateData>& geometry_shader() const { return m_geometry_shader; }

    private:

        std::optional<ShaderCreateData> m_vertex_shader{};
        std::optional<ShaderCreateData> m_fragment_shader{};
        std::optional<ShaderCreateData> m_geometry_shader{};

        IRenderer* const m_renderer = nullptr;

    };
}