#pragma once

#include "rendering/pipeline.h"

#include <memory>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Pipeline_Vulkan : public IPipeline
    {
    public:

        Pipeline_Vulkan() = delete;

        static std::shared_ptr<Pipeline_Vulkan> create();

    private:

        Pipeline_Vulkan(const vk::Pipeline& pipeline);

    private:

        vk::Pipeline m_pipeline;

    };
}