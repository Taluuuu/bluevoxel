#include "renderer_vulkan.h"

#include "core/game_info.h"
#include "core/types.h"
#include "windowing/window.h"

// GLFW for Vulkan
// TODO: this file should not assume Vulkan is used with GLFW.
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <set>

PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger )
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT messenger, 
    VkAllocationCallbacks const* pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    std::cerr << "validation layer: " << callback_data->pMessage << std::endl;
    return VK_FALSE;
}

namespace engine
{
    Renderer_Vulkan::Renderer_Vulkan(const GameInfo& game_info, const IWindow& window)
    {
        // TODO: Make sure to destroy the instance if setup_debug_messenger fails.
        create_instance(game_info.game_name.data(), game_info.engine_name.data());
        setup_debug_messenger();
        create_surface(window);
        pick_physical_device();
        create_logical_device();
    }

    Renderer_Vulkan::~Renderer_Vulkan()
    {
        m_device.destroy();
        m_instance.destroySurfaceKHR(m_surface);
        m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger);
        m_instance.destroy();
    }

    void Renderer_Vulkan::create_instance(const char* game_name, const char* engine_name)
    {
        if (m_enable_validation_layers && !validation_layers_are_supported())
            throw std::runtime_error("Validation layers are required but not available.");

        vk::ApplicationInfo app_info(
            game_name, 
            VK_MAKE_API_VERSION(0, 0, 0, 1),
            engine_name,
            VK_MAKE_API_VERSION(0, 0, 0, 1),
            VK_API_VERSION_1_0
        );

        auto extensions = get_required_extensions();
        vk::InstanceCreateInfo create_info({}, 
            &app_info, 
            0, nullptr, 
            static_cast<u32>(extensions.size()), extensions.data(), 
            nullptr
        );

        vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info;
        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();

            populate_debug_messenger_create_info(debug_messenger_create_info);
            create_info.pNext = &debug_messenger_create_info;
        }

        m_instance = vk::createInstance(create_info);
    }

    void Renderer_Vulkan::setup_debug_messenger()
    {
        if (!m_enable_validation_layers)
            return;

        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        if (!pfnVkCreateDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(m_instance.getProcAddr( "vkDestroyDebugUtilsMessengerEXT"));
        if (!pfnVkDestroyDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

        vk::DebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);

        m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(create_info);
    }

    void Renderer_Vulkan::create_surface(const IWindow& window)
    {
        VkSurfaceKHR surface = m_surface;
        auto result = glfwCreateWindowSurface(
            m_instance, 
            reinterpret_cast<GLFWwindow*>(window.wrapped_window_handle()), 
            nullptr, 
            &surface);

        m_surface = surface;

        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create GLFW window surface.");
    }

    void Renderer_Vulkan::pick_physical_device()
    {
        std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
        if (devices.empty())
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");

        for (const auto& device : devices)
        {
            if (is_device_suitable(device))
            {
                m_physical_device = device;
                break;
            }
        }

        if (!m_physical_device)
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");
    }

    void Renderer_Vulkan::create_logical_device()
    {
        QueueFamilyIndices indices = find_queue_families(m_physical_device);

        // TODO: .value() could throw - there might be more instances of this in this file.
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = 
        {
            indices.graphics_family.value(),
            indices.present_family.value()
        };

        f32 queue_priority = 1.0f;
        for (u32 queue_family : unique_queue_families)
        {
            vk::DeviceQueueCreateInfo queue_create_info({}, 
                queue_family, 1, &queue_priority);

            queue_create_infos.push_back(queue_create_info);
        }
        
        vk::PhysicalDeviceFeatures device_features;

        vk::DeviceCreateInfo create_info({}, 
            static_cast<u32>(queue_create_infos.size()), queue_create_infos.data(), 
            0, nullptr, 
            0, nullptr, 
            &device_features, 
            nullptr);

        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();
        }

        m_device        = m_physical_device.createDevice(create_info);
        m_queue         = m_device.getQueue(indices.graphics_family.value(), 0);
        m_present_queue = m_device.getQueue(indices.present_family.value(), 0);
    }

    std::vector<const char*> Renderer_Vulkan::get_required_extensions() const
    {
        u32 glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        if (m_enable_validation_layers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    bool Renderer_Vulkan::validation_layers_are_supported() const
    {
        auto available_layers = vk::enumerateInstanceLayerProperties();

        for (const char* layer_name : m_validation_layers)
        {
            bool layer_found = false;

            for (const auto& layer_properties : available_layers)
            {
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
                return false;
        }

        return true;

    }

    void Renderer_Vulkan::populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) const
    {
        const vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags = 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        const vk::DebugUtilsMessageTypeFlagsEXT message_type_flags = 
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

        debug_utils_messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT({},
            message_severity_flags,
            message_type_flags,
            debug_callback
        );
    }
    
    bool Renderer_Vulkan::is_device_suitable(const vk::PhysicalDevice& device) const
    {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
        // We can evaluate with more criterias which devices we want

        QueueFamilyIndices indices = find_queue_families(device);
        return indices.is_complete();
    }

    Renderer_Vulkan::QueueFamilyIndices Renderer_Vulkan::find_queue_families(const vk::PhysicalDevice& device) const
    {
        QueueFamilyIndices indices;

        auto queue_families = device.getQueueFamilyProperties();

        u32 i = 0;
        for (const auto& queue_family : queue_families)
        {
            if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
                indices.graphics_family = i;

            if (device.getSurfaceSupportKHR(i, m_surface))
                indices.present_family = i;

            if (indices.is_complete())
                break;

            i++;
        }

        return indices;
    }
}