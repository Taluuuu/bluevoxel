#pragma once

#include <memory>
#include <string_view>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Shader_Vulkan
    {
    public:

        Shader_Vulkan() = delete;

        static std::unique_ptr<Shader_Vulkan> create(
            const vk::Device& device, 
            const std::string& path);

    private:

        Shader_Vulkan(const vk::ShaderModule& shader);

    private:

        vk::ShaderModule m_shader = nullptr;

    };
}