#include "shader_vulkan.h"

#include "core/utils.h"
#include "core/log.h"
#include "renderer_vulkan.h"

namespace engine
{
    std::optional<Shader_Vulkan> Shader_Vulkan::create(
        ShaderStage stage, 
        const std::string& path, 
        const Renderer_Vulkan& renderer)
    {
        auto code = utils::read_file(path);
        if (!code.has_value())
        {
            log::error("Failed to load shader at path: '{}'", path);
            return std::nullopt;
        }

        vk::ShaderModuleCreateInfo create_info({}, 
            code->size(),
            reinterpret_cast<const u32*>(code->data())
        );

        vk::ShaderModule shader = nullptr;
        try
        {
            shader = renderer.device().createShaderModule(create_info);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create shader at path '{}': {}", path, e.what());
            return std::nullopt;
        }

        return Shader_Vulkan(stage, shader);
    }

    static vk::ShaderStageFlagBits shader_stage_bits(engine::ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:   return vk::ShaderStageFlagBits::eVertex;
        case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
        case ShaderStage::Geometry: return vk::ShaderStageFlagBits::eGeometry;
        }

        return {};
    }

    vk::PipelineShaderStageCreateInfo Shader_Vulkan::make_pipeline_shader_stage_create_info() const
    {
        return vk::PipelineShaderStageCreateInfo({},
            shader_stage_bits(m_stage),
            m_shader,
            "main",
            nullptr
        );
    }

    ShaderStage Shader_Vulkan::stage() const
    {
        return m_stage;
    }

    Shader_Vulkan::Shader_Vulkan(ShaderStage stage, const vk::ShaderModule& shader)
        : m_stage(stage)
        , m_shader(shader) {}
}