#pragma once

#include "rendering/pipeline.h"
#include "rendering/renderer_enums.h"
#include "shader_vulkan.h" // TODO: Check if it's better to have this be a forward declaration

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Renderer_Vulkan;

    class Pipeline_Vulkan : public IPipeline
    {
    public:

        Pipeline_Vulkan(const std::shared_ptr<Renderer_Vulkan>& renderer);
        Pipeline_Vulkan(const Pipeline_Vulkan&) = delete;
        Pipeline_Vulkan(Pipeline_Vulkan&& other) = delete;
        ~Pipeline_Vulkan();

        void reset(const PipelineFactory& factory);

        // IPipeline interface
        // ...

        const vk::Pipeline& pipeline_handle() const { return m_pipeline_handle; }

    private:

        void register_shader(std::unique_ptr<Shader_Vulkan>&& shader);

    private:

        std::vector<std::unique_ptr<Shader_Vulkan>> m_shaders;
        vk::Pipeline m_pipeline_handle = nullptr;
        vk::PipelineLayout m_layout = nullptr;

        std::shared_ptr<Renderer_Vulkan> const m_renderer = nullptr;

    };
}