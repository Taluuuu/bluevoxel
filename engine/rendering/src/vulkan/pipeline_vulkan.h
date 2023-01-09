#pragma once

#include "rendering/pipeline.h"

#include <memory>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Pipeline_Vulkan : public IPipeline
    {
    public:

        Pipeline_Vulkan(const vk::Device& device);

        // IPipeline interface
        virtual IPipeline& add_shader(engine::ShaderType type, const std::string& path) override;
        virtual IPipeline& compile() override;
        virtual bool is_ready() const override;

    private:

        vk::Device m_device = nullptr;
        vk::Pipeline m_pipeline = nullptr;

    };
}