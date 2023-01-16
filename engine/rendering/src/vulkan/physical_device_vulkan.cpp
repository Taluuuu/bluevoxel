#include "physical_device_vulkan.h"

#include "core/log.h"
#include "renderer_vulkan.h"

#include <set>
#include <string>
#include <vector>

namespace engine
{
    std::unique_ptr<PhysicalDevice_Vulkan> PhysicalDevice_Vulkan::create(const Renderer_Vulkan& renderer)
    {
        auto instance = renderer.instance();

        std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
        if (devices.empty())
        {
            log::error("Failed to find GPUs with Vulkan support.");
            return nullptr;
        }

        vk::PhysicalDevice physical_device = nullptr;
        for (const auto& device : devices)
        {
            if (is_device_suitable(device, renderer))
            {
                physical_device = device;
                break;
            }
        }

        if (!physical_device)
        {
            log::error("Failed to find GPUs with Vulkan support.");
            return nullptr;
        }

        return std::unique_ptr<PhysicalDevice_Vulkan>(new PhysicalDevice_Vulkan(physical_device));
    }

    vk::Device PhysicalDevice_Vulkan::create_device(const vk::DeviceCreateInfo& create_info) const
    {
        return m_physical_device_handle.createDevice(create_info);
    }

    PhysicalDevice_Vulkan::PhysicalDevice_Vulkan(const vk::PhysicalDevice &physical_device_handle)
        : m_physical_device_handle(physical_device_handle)
    {}

    bool PhysicalDevice_Vulkan::is_device_suitable(const vk::PhysicalDevice& device, const Renderer_Vulkan& renderer)
    {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
        // We can evaluate with more criterias which devices we want

        bool extensions_supported = device_supports_extensions(device, renderer);

        auto surface = renderer.surface();
        bool is_swapchain_adequate = false;
        if (extensions_supported)
        {
            auto swapchain_support = renderer.query_swapchain_support(device);
            is_swapchain_adequate = swapchain_support.is_adequate();
        }

        auto indices = renderer.find_queue_families(device);
        return indices.is_complete() && extensions_supported && is_swapchain_adequate;
    }

    bool PhysicalDevice_Vulkan::device_supports_extensions(const vk::PhysicalDevice &device, const Renderer_Vulkan& renderer)
    {
        auto available_extensions = device.enumerateDeviceExtensionProperties();
        const auto& device_extensions = renderer.device_extensions();
        std::set<std::string> required_extensions(
            device_extensions.begin(), device_extensions.end());

        for (const auto& extension : available_extensions)
            required_extensions.erase(extension.extensionName);

        return required_extensions.empty();
    }
}