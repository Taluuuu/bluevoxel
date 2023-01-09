#include "shader_vulkan.h"

#include "core/utils.h"
#include "core/log.h"

namespace engine
{
    std::unique_ptr<Shader_Vulkan> Shader_Vulkan::create(
        ShaderType type,
        const vk::Device& device, 
        const std::string& path)
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
            shader = device.createShaderModule(create_info);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create shader at path '{}': {}", path, e.what());
            return nullptr;
        }

        vk::ShaderStageFlagBits shader_stage = {};
        switch (type)
        {
        case ShaderType::Vertex:   shader_stage = vk::ShaderStageFlagBits::eVertex;   break;
        case ShaderType::Fragment: shader_stage = vk::ShaderStageFlagBits::eFragment; break;
        case ShaderType::Geometry: shader_stage = vk::ShaderStageFlagBits::eGeometry; break;
        }

        vk::PipelineShaderStageCreateInfo vert_shader_stage_create_info({},
            shader_stage,
            shader,
            "main",
            nullptr // SpecializationInfo, used to set constants at runtime
        );

        return std::make_unique<Shader_Vulkan>(shader);
    }

    Shader_Vulkan::Shader_Vulkan(const vk::ShaderModule& shader)
        : m_shader(shader) {}
}