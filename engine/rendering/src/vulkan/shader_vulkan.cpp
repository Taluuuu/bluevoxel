#include "shader_vulkan.h"

#include "core/utils.h"
#include "core/log.h"
#include "device_vulkan.h"
#include "renderer_vulkan.h"

namespace engine
{
    std::unique_ptr<Shader_Vulkan> Shader_Vulkan::create(
        ShaderStage stage, 
        const std::string& path, 
        const Renderer_Vulkan& renderer)
    {
        auto code = utils::read_file(path);
        if (!code.has_value())
        {
            log::error("Failed to load shader at path: '{}'", path);
            return nullptr;
        }

        vk::ShaderModuleCreateInfo create_info({}, 
            code->size(),
            reinterpret_cast<const u32*>(code->data())
        );

        vk::ShaderModule shader = nullptr;
        try
        {
            shader = renderer.device().handle().createShaderModule(create_info);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create shader at path '{}': {}", path, e.what());
            return nullptr;
        }

        return std::unique_ptr<Shader_Vulkan>(new Shader_Vulkan(stage, shader, renderer));
    }

    Shader_Vulkan::Shader_Vulkan(Shader_Vulkan&& other)
        : m_shader_handle(other.m_shader_handle)
        , m_stage(other.m_stage)
        , m_renderer(other.m_renderer)
    {
        m_shader_handle = nullptr;
    }

    Shader_Vulkan::~Shader_Vulkan()
    {
        if (m_shader_handle)
            m_renderer->device().handle().destroyShaderModule(m_shader_handle);
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
            m_shader_handle,
            "main",
            nullptr
        );
    }

    ShaderStage Shader_Vulkan::stage() const
    {
        return m_stage;
    }

    Shader_Vulkan::Shader_Vulkan(ShaderStage stage, const vk::ShaderModule& shader, const Renderer_Vulkan& renderer)
        : m_stage(stage)
        , m_shader_handle(shader)
        , m_renderer(&renderer) {}
}