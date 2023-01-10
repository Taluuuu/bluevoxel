#pragma once

#include "rendering/pipeline.h"
#include "rendering/renderer_enums.h"
#include "shader_vulkan.h" // TODO: Check if it's better to have this be a forward declaration

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Pipeline_Vulkan : public IPipeline
    {
    public:

        Pipeline_Vulkan(const vk::Device& device);
        Pipeline_Vulkan(const Pipeline_Vulkan&) = delete;
        Pipeline_Vulkan(Pipeline_Vulkan&& other) = delete;

        // IPipeline interface
        virtual IPipeline& add_shader(ShaderStage type, const std::string& path) override;
        virtual IPipeline& compile() override;
        virtual bool is_ready() const override;

    private:

        void register_shader(const Shader_Vulkan& shader);

    private:

        std::vector<std::optional<Shader_Vulkan>> m_shaders;

        vk::Device m_device = nullptr;
        vk::Pipeline m_pipeline = nullptr;

        bool m_has_compiled = false;

    };
}