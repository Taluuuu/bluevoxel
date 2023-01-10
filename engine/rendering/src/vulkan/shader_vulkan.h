#pragma once

#include "rendering/renderer_enums.h"

#include <memory>
#include <optional>
#include <string_view>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Shader_Vulkan
    {
    public:

        Shader_Vulkan() = delete;

        static std::optional<Shader_Vulkan> create(
            ShaderStage stage,
            const vk::Device& device, 
            const std::string& path);

        vk::PipelineShaderStageCreateInfo make_pipeline_shader_stage_create_info() const;

        ShaderStage stage() const;

    private:

        Shader_Vulkan(ShaderStage stage, const vk::ShaderModule& shader);

    private:

        vk::ShaderModule m_shader = nullptr;
        ShaderStage m_stage;

    };
}