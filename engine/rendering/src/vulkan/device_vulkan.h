#pragma once

#include <vulkan/vulkan.hpp>

namespace engine
{
    class Renderer_Vulkan;

    class Device_Vulkan
    {
    public:

        static std::unique_ptr<Device_Vulkan> create(
            const Renderer_Vulkan& renderer);

        Device_Vulkan(const Device_Vulkan&) = delete;
        Device_Vulkan(Device_Vulkan&&) = delete;
        ~Device_Vulkan() = default;

        const vk::PhysicalDevice& physical_device_handle() const { return m_physical_device_handle; }

        vk::Device create_device(const vk::DeviceCreateInfo& create_info) const;

    private:

        Device_Vulkan(
            const vk::PhysicalDevice& physical_device_handle,
            const vk::Device& logical_device_handle);

    private:

        vk::PhysicalDevice m_physical_device_handle = nullptr;
        vk::Device         m_logical_device_handle  = nullptr;

    };
}