#pragma once

#include "rendering/pipeline.h"
#include "rendering/renderer_enums.h"

#include <array>
#include <magic_enum.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Pipeline_Vulkan : public IPipeline
    {
    public:

        Pipeline_Vulkan(const vk::Device& device);

        // IPipeline interface
        virtual IPipeline& add_shader(ShaderType type, const std::string& path) override;
        virtual IPipeline& compile() override;
        virtual bool is_ready() const override;

    private:

        std::array<std::unique_ptr<Shader_Vulkan>, magic_enum::enum_count<ShaderType>> test;

        vk::Device m_device = nullptr;
        vk::Pipeline m_pipeline = nullptr;

    };
}