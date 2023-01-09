#include "pipeline_vulkan.h"

#include "shader_vulkan.h"

namespace engine
{
    Pipeline_Vulkan::Pipeline_Vulkan(const vk::Device &device)
        : m_device(device)
    {
        assert(device);
    }

    IPipeline &Pipeline_Vulkan::add_shader(engine::ShaderType type, const std::string &path)
    {
        Shader_Vulkan::create(m_device, path);

        return *this;
    }

    IPipeline &Pipeline_Vulkan::compile()
    {
        return *this;
    }
    
    bool Pipeline_Vulkan::is_ready() const
    {
        return false;
    }
}