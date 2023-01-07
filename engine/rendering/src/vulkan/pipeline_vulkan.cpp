#include "pipeline_vulkan.h"

namespace engine
{
    std::shared_ptr<Pipeline_Vulkan> Pipeline_Vulkan::create()
    {
        return std::make_shared<Pipeline_Vulkan>(pipeline);
    }

    Pipeline_Vulkan::Pipeline_Vulkan(const vk::Pipeline& pipeline)
        : m_pipeline(pipeline) {}
}