#include "device_vulkan.h"

#include "renderer_vulkan.h"
#include "surface_vulkan.h"

#include "core/log.h"

#include <set>
#include <string>
#include <vector>

namespace engine
{
    static bool physical_device_supports_extensions(const vk::PhysicalDevice& physical_device, const Renderer_Vulkan& renderer)
    {
        auto available_extensions = physical_device.enumerateDeviceExtensionProperties();
        const auto& device_extensions = renderer.device_extensions();
        std::set<std::string> required_extensions(
            device_extensions.begin(), device_extensions.end());

        for (const auto& extension : available_extensions)
            required_extensions.erase(extension.extensionName);

        return required_extensions.empty();
    }

    static bool is_physical_device_suitable(const vk::PhysicalDevice& physical_device, const Renderer_Vulkan& renderer)
    {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
        // We can evaluate with more criterias which devices we want

        bool extensions_supported = physical_device_supports_extensions(physical_device, renderer);

        const auto& surface = renderer.surface();
        bool is_swapchain_adequate = false;
        if (extensions_supported)
        {
            auto swapchain_support = surface.query_swapchain_support(physical_device);
            is_swapchain_adequate = swapchain_support.is_adequate();
        }

        auto indices = renderer.find_queue_families(physical_device);
        return indices.is_complete() && extensions_supported && is_swapchain_adequate;
    }

    static vk::PhysicalDevice select_physical_device(const Renderer_Vulkan& renderer)
    {
        auto instance = renderer.instance();

        std::vector<vk::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();
        if (physical_devices.empty())
        {
            log::error("Failed to find GPUs with Vulkan support.");
            return nullptr;
        }

        vk::PhysicalDevice selected_physical_device = nullptr;
        for (const auto& physical_device : physical_devices)
        {
            if (is_physical_device_suitable(physical_device, renderer))
            {
                selected_physical_device = physical_device;
                break;
            }
        }

        return selected_physical_device;
    }

    vk::Device create_logical_device(
        const vk::PhysicalDevice& physical_device, 
        const Renderer_Vulkan& renderer,
        vk::Queue& graphics_queue,
        vk::Queue& present_queue)
    {
        auto indices = renderer.find_queue_families(physical_device);

        // TODO: .value() could throw
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families =
        {
            indices.graphics_family.value(),
            indices.present_family.value()};

        f32 queue_priority = 1.0f;
        for (u32 queue_family : unique_queue_families)
        {
            vk::DeviceQueueCreateInfo queue_create_info({}, 
                queue_family, 1, &queue_priority);

            queue_create_infos.push_back(queue_create_info);
        }
        
        vk::PhysicalDeviceFeatures device_features;

        auto device_extensions = renderer.device_extensions();

        vk::DeviceCreateInfo create_info({}, 
            static_cast<u32>(queue_create_infos.size()), queue_create_infos.data(), // Queue create infos
            0,                                           nullptr,                   // Validation layers
            static_cast<u32>(device_extensions.size()),  device_extensions.data(),  // Device extensions
            &device_features, 
            nullptr);

        if (renderer.enable_validation_layers())
        {
            const auto& validation_layers = renderer.validation_layers();
            create_info.enabledLayerCount = static_cast<u32>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        }

        try
        {
            auto logical_device = physical_device.createDevice(create_info);
            graphics_queue = logical_device.getQueue(indices.graphics_family.value(), 0);
            present_queue  = logical_device.getQueue(indices.present_family.value(),  0);
            return logical_device;
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create logical device: {}", e.what());
            return nullptr;
        }
    }

    std::unique_ptr<Device_Vulkan> Device_Vulkan::create(const Renderer_Vulkan& renderer)
    {
        auto physical_device = select_physical_device(renderer);
        if (!physical_device)
        {
            log::error("Failed to find GPUs with Vulkan support.");
            return nullptr;
        }

        vk::Queue graphics_queue, present_queue;
        auto logical_device = create_logical_device(physical_device, renderer, graphics_queue, present_queue);
        if (!logical_device)
            return nullptr;
        // TODO: error msg

        return std::unique_ptr<Device_Vulkan>(new Device_Vulkan(physical_device, logical_device, graphics_queue, present_queue));
    }

    vk::Pipeline Device_Vulkan::create_pipeline(const vk::GraphicsPipelineCreateInfo& create_info) const
    {
        // TODO: createGraphicsPipeline can throw
        auto result = m_logical_device_handle.createGraphicsPipeline(nullptr, create_info);
        if (result.result == vk::Result::eSuccess)
            return result.value;

        return nullptr;
    }

    vk::PipelineLayout Device_Vulkan::create_pipeline_layout(const vk::PipelineLayoutCreateInfo& create_info) const
    {
        return m_logical_device_handle.createPipelineLayout(create_info);
    }

    void Device_Vulkan::destroy_pipeline(const vk::Pipeline& pipeline, const vk::PipelineLayout& pipeline_layout) const
    {
        if (pipeline)
            m_logical_device_handle.destroyPipeline(pipeline);

        if (pipeline_layout)
            m_logical_device_handle.destroyPipelineLayout(pipeline_layout);
    }

    Device_Vulkan::Device_Vulkan(const vk::PhysicalDevice& physical_device_handle, const vk::Device& logical_device_handle, const vk::Queue& graphics_queue, const vk::Queue& present_queue)
        : m_physical_device_handle(physical_device_handle)
        , m_logical_device_handle(logical_device_handle)
        , m_graphics_queue(graphics_queue)
        , m_present_queue(present_queue)
    {}
}