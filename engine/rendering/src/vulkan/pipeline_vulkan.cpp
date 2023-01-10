#include "pipeline_vulkan.h"

#include "core/log.h"
#include "core/utils.h"
#include "shader_vulkan.h"

#include <magic_enum.hpp>

namespace engine
{
    Pipeline_Vulkan::Pipeline_Vulkan(const vk::Device &device)
        : m_device(device)
    {
        assert(device);

        m_shaders.resize(magic_enum::enum_count<ShaderStage>(), std::nullopt);
    }

    IPipeline& Pipeline_Vulkan::add_shader(engine::ShaderStage stage, const std::string& path)
    {
        auto shader = Shader_Vulkan::create(stage, m_device, path);
        if (shader)
            register_shader(*shader);

        return *this;
    }

    IPipeline& Pipeline_Vulkan::compile()
    {
        // Create shader stage create info array
        std::vector<vk::PipelineShaderStageCreateInfo> shader_create_infos;
        shader_create_infos.reserve(m_shaders.size());
        for (const auto& shader : m_shaders)
        {
            if (shader.has_value())
            {
                shader_create_infos.push_back(
                    shader->make_pipeline_shader_stage_create_info());
            }
        }

        

        return *this;
    }
    
    bool Pipeline_Vulkan::is_ready() const
    {
        return m_has_compiled;
    }

    void Pipeline_Vulkan::register_shader(const Shader_Vulkan& shader)
    {
        auto stage_index = magic_enum::enum_index(shader.stage());
        if (!stage_index.has_value())
            return;

        m_shaders[*stage_index] = shader;
    }
}