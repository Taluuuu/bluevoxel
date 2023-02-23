#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace engine
{
    class Renderer_Vulkan;

    class Device_Vulkan : public std::enable_shared_from_this<Device_Vulkan>
    {
    public:

        static std::shared_ptr<Device_Vulkan> create(const std::shared_ptr<Renderer_Vulkan>& renderer);

        Device_Vulkan(const Device_Vulkan&) = delete;
        Device_Vulkan(Device_Vulkan&&) = delete;
        ~Device_Vulkan() = default;

        void wait_idle() const { m_logical_device_handle.waitIdle(); }

        // TODO: Remove the physical device handle
        const vk::PhysicalDevice& physical_device_handle() const { return m_physical_device_handle; }
        const vk::Device&         handle()                 const { return m_logical_device_handle;  }
        const vk::Queue&          graphics_queue()         const { return m_graphics_queue;         }
        const vk::Queue&          present_queue()          const { return m_present_queue;          }

        // Pipeline
        vk::Pipeline       create_pipeline(const vk::GraphicsPipelineCreateInfo& create_info) const;
        vk::PipelineLayout create_pipeline_layout(const vk::PipelineLayoutCreateInfo& create_info) const;
        void               destroy_pipeline(const vk::Pipeline& pipeline, const vk::PipelineLayout& pipeline_layout) const;

    private:

        Device_Vulkan(
            const std::shared_ptr<Renderer_Vulkan>& renderer,
            const vk::PhysicalDevice& physical_device_handle,
            const vk::Device& logical_device_handle,
            const vk::Queue& graphics_queue,
            const vk::Queue& present_queue);

    private:

        vk::PhysicalDevice m_physical_device_handle = nullptr;
        vk::Device         m_logical_device_handle  = nullptr;

        vk::Queue          m_graphics_queue         = nullptr;
        vk::Queue          m_present_queue          = nullptr;

        std::shared_ptr<Renderer_Vulkan> const m_renderer = nullptr;

    };
}