#pragma once

#include <vulkan/vulkan.hpp>

namespace engine
{
    class Renderer_Vulkan;

    class PhysicalDevice_Vulkan
    {
    public:

        static std::unique_ptr<PhysicalDevice_Vulkan> create(
            const Renderer_Vulkan& renderer);

        PhysicalDevice_Vulkan(const PhysicalDevice_Vulkan&) = delete;
        PhysicalDevice_Vulkan(PhysicalDevice_Vulkan&&) = delete;
        ~PhysicalDevice_Vulkan() = default;

        const vk::PhysicalDevice& physical_device_handle() const { return m_physical_device_handle; }

        vk::Device create_device(const vk::DeviceCreateInfo& create_info) const;

    private:

        PhysicalDevice_Vulkan(const vk::PhysicalDevice& physical_device_handle);

        static bool is_device_suitable(const vk::PhysicalDevice& device, const Renderer_Vulkan& renderer);
        static bool device_supports_extensions(const vk::PhysicalDevice& device, const Renderer_Vulkan& renderer);

    private:

        vk::PhysicalDevice m_physical_device_handle = nullptr;

    };
}