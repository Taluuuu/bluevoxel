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

        virtual ~IPipeline() = default;

        [[nodiscard]] virtual bool is_ready() const = 0;

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