#pragma once

#include "rendering/renderer_enums.h"

#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Renderer_Vulkan;

    class Shader_Vulkan
    {
    public:

        static std::unique_ptr<Shader_Vulkan> create(
            ShaderStage stage, 
            const std::string& path,
            const Renderer_Vulkan& renderer);

        Shader_Vulkan(const Shader_Vulkan&) = delete;
        Shader_Vulkan(Shader_Vulkan&& other);
        ~Shader_Vulkan();

        vk::PipelineShaderStageCreateInfo make_pipeline_shader_stage_create_info() const;

        ShaderStage stage() const;

    private:

        Shader_Vulkan(ShaderStage stage, const vk::ShaderModule& shader, const Renderer_Vulkan& renderer);

    private:

        vk::ShaderModule m_shader_handle = nullptr;
        ShaderStage m_stage;

        const Renderer_Vulkan* const m_renderer = nullptr;

    };
}