#pragma once

#include "rendering/pipeline.h"
#include "rendering/renderer_enums.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace engine
{
    class Renderer_Vulkan;

    class Pipeline_Vulkan : public IPipeline
    {
    public:

        static std::shared_ptr<Pipeline_Vulkan> create(
            const std::shared_ptr<Renderer_Vulkan>& renderer,
            const PipelineCreateData& create_data);

        Pipeline_Vulkan(const Pipeline_Vulkan&) = delete;
        Pipeline_Vulkan(Pipeline_Vulkan&& other) = delete;
        ~Pipeline_Vulkan() override;

        // IPipeline interface
        [[nodiscard]] bool is_ready() const override;

        [[nodiscard]] const vk::Pipeline& handle() const { return m_pipeline; }

    private:

        Pipeline_Vulkan(
            const std::shared_ptr<Renderer_Vulkan>& renderer,
            const vk::Pipeline& pipeline,
            const vk::PipelineLayout& pipeline_layout);

    private:

        vk::Pipeline m_pipeline = nullptr;
        vk::PipelineLayout m_layout = nullptr;

        std::shared_ptr<Renderer_Vulkan> const m_renderer = nullptr;

    };
}