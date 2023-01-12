#include "pipeline_vulkan.h"

#include "core/log.h"
#include "core/utils.h"
#include "shader_vulkan.h"

#include <magic_enum.hpp>

namespace engine
{
    Pipeline_Vulkan::Pipeline_Vulkan(const vk::Device &device)
        : m_device(device)
    {
        assert(device);

        m_shaders.resize(magic_enum::enum_count<ShaderStage>(), std::nullopt);
    }

    IPipeline& Pipeline_Vulkan::add_shader(engine::ShaderStage stage, const std::string& path)
    {
        auto shader = Shader_Vulkan::create(stage, m_device, path);
        if (shader)
            register_shader(*shader);

        return *this;
    }

    IPipeline& Pipeline_Vulkan::compile()
    {
        m_has_compiled = false;

        // Create shader stage create info array
        std::vector<vk::PipelineShaderStageCreateInfo> shader_create_infos;
        bool has_vertex_shader = false, has_fragment_shader = false;
        shader_create_infos.reserve(m_shaders.size());
        for (const auto& shader : m_shaders)
        {
            if (shader.has_value())
            {
                shader_create_infos.push_back(
                    shader->make_pipeline_shader_stage_create_info());

                has_vertex_shader   = has_vertex_shader   || (shader->stage() == ShaderStage::Vertex);
                has_fragment_shader = has_fragment_shader || (shader->stage() == ShaderStage::Fragment);
            }
        }

        // Make sure there is at least a vertex and fragment shader
        if (!has_fragment_shader || !has_vertex_shader)
        {
            log::error("Failed to create pipeline. Missing vertex shader and/or fragment shader.");
            return *this;
        }
        
        // Vertex input format
        // Bindings   : Spacing between data and whether the data is per-vertex or 
        //              per-instance
        // Attributes : Type of the attributes passed to the vertex shader, which 
        //              binding to load them from and at which offset
        vk::PipelineVertexInputStateCreateInfo vertex_input_info({},
            0, nullptr, // Vertex binding descriptions
            0, nullptr  // Vertex attribute descriptions
        );

        vk::PipelineInputAssemblyStateCreateInfo assembly_info({},
            vk::PrimitiveTopology::eTriangleList,
            false
        );

        // vk::Viewport viewport(
        //     0.0f, 0.0f,
        //     static_cast<f32>(m_swapchain_extent.width), static_cast<f32>(m_swapchain_extent.height),
        //     0.0f, 1.0f
        // );

        // vk::Rect2D scissor(
        //     { 0, 0 },
        //     m_swapchain_extent
        // );

        // std::vector<vk::DynamicState> dynamic_states = 
        // {
        //     vk::DynamicState::eViewport,
        //     vk::DynamicState::eScissor
        // };
        // vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info({},
        //     static_cast<u32>(dynamic_states.size()),
        //     dynamic_states.data()
        // );

        // vk::PipelineViewportStateCreateInfo viewport_state_create_info({},
        //     1, &viewport, 
        //     1, &scissor
        // );

        // vk::PipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info({},
        //     VK_FALSE, // Might be useful for shadow mapping
        //     VK_FALSE,
        //     vk::PolygonMode::eFill,
        //     vk::CullModeFlagBits::eBack,
        //     vk::FrontFace::eClockwise,
        //     VK_FALSE, 0.0f, 0.0f, 0.0f,
        //     1.0f
        // );

        // // Disabled multisampling
        // vk::PipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info({},
        //     vk::SampleCountFlagBits::e1,
        //     VK_FALSE,
        //     1.0f, nullptr, VK_FALSE, VK_FALSE // This line is optional as multisampling is disabled
        // );

        // // Search alpha blending for transparency
        // vk::PipelineColorBlendAttachmentState pipeline_color_blend_attachment_state(
        //     VK_FALSE,
        //     // The following is optional as blending is disabled
        //     // Color blend
        //     vk::BlendFactor::eOne,
        //     vk::BlendFactor::eZero,
        //     vk::BlendOp::eAdd,
        //     // Alpha blend
        //     vk::BlendFactor::eOne,
        //     vk::BlendFactor::eZero,
        //     vk::BlendOp::eAdd,
        //     // Not optional
        //     vk::ColorComponentFlagBits::eR |
        //     vk::ColorComponentFlagBits::eG |
        //     vk::ColorComponentFlagBits::eB |
        //     vk::ColorComponentFlagBits::eA
        // );

        // vk::PipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info({},
        //     VK_FALSE,
        //     vk::LogicOp::eCopy,
        //     1, &pipeline_color_blend_attachment_state,
        //     { 0.0f, 0.0f, 0.0f, 0.0f }
        // );

        // // Will be used to set uniforms
        // vk::PipelineLayoutCreateInfo pipeline_layout_create_info({},
        //     0, nullptr,
        //     0, nullptr
        // );
        // m_pipeline_layout = m_device.createPipelineLayout(pipeline_layout_create_info);

        // // TODO: Rework naming for create infos, currently a mess
        // vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info({},
        //     2, shader_stages_create_infos,
        //     &vertex_input_info,
        //     &assembly_info,
        //     nullptr,
        //     &viewport_state_create_info,
        //     &pipeline_rasterization_state_create_info,
        //     &pipeline_multisample_state_create_info,
        //     nullptr,
        //     &pipeline_color_blend_state_create_info,
        //     &pipeline_dynamic_state_create_info,
        //     m_pipeline_layout,
        //     m_render_pass,
        //     0,
        //     nullptr,
        //     -1
        // );

        return *this;
    }
    
    bool Pipeline_Vulkan::is_ready() const
    {
        return m_has_compiled;
    }

    void Pipeline_Vulkan::register_shader(const Shader_Vulkan& shader)
    {
        auto stage_index = magic_enum::enum_index(shader.stage());
        if (!stage_index.has_value())
            return;

        m_shaders[*stage_index] = shader;
    }
}