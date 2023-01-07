#include "shader_vulkan.h"

#include "core/utils.h"
#include "core/log.h"

namespace engine
{
    std::unique_ptr<Shader_Vulkan> Shader_Vulkan::create(
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

        try
        {
            auto shader = device.createShaderModule(create_info);
            return std::make_unique<Shader_Vulkan>(shader);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create shader at path '{}': {}", path, e.what());
            return nullptr;
        }
    }

    Shader_Vulkan::Shader_Vulkan(const vk::ShaderModule& shader)
        : m_shader(shader) {}
}