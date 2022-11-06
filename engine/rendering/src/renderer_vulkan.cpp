#include "renderer_vulkan.h"

#include "core/app_info.h"
#include "core/types.h"

#include <cstring>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

namespace engine
{
    static bool are_validation_layers_supported(const std::vector<const char*>& validation_layers)
    {
        // Get available validation layers
        u32 layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        // Check if requested validation layers are available
        for (const auto& layer_name : validation_layers)
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

    static std::vector<const char*> get_required_extensions(bool enable_validation_layers)
    {
        // GLFW extensions
        u32 glfw_extension_count = 0;
        auto glfw_extensions = 
            glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(
            glfw_extensions, glfw_extensions + glfw_extension_count);

        if (enable_validation_layers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::cout << "[Error] validation layer: " << pCallbackData->pMessage << "\n";
        return VK_FALSE;
    }

    static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = 
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
        };
    }


    Renderer_Vulkan::Renderer_Vulkan(const AppInfo& app_info, const IWindow& window)
    {
        VkApplicationInfo vulkan_app_info
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = app_info.app_name.data(),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = app_info.engine_name.data(),
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0,
        };
        
        VkInstanceCreateInfo create_info
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &vulkan_app_info,
        };

        auto extensions = get_required_extensions(enable_validation_layers);
        create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
        if (enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();

            populate_debug_messenger_create_info(debug_create_info);
            create_info.pNext = &debug_create_info;
        }
        else
        {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create vulkan instance");
        }

        if (enable_validation_layers)
            setup_debug_messenger();
    }

    static VkResult create_debug_utils_messenger_ext(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func == nullptr)
            return VK_ERROR_EXTENSION_NOT_PRESENT;

        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }

    static void destroy_debug_utils_messenger_ext(
        VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    Renderer_Vulkan::~Renderer_Vulkan()
    {
        assert(m_instance != nullptr);

        if (enable_validation_layers)
            destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, nullptr);

        vkDestroyInstance(m_instance, nullptr);
    }

    void Renderer_Vulkan::setup_debug_messenger()
    {
        if (!enable_validation_layers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);

        if (create_debug_utils_messenger_ext(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to setup debug messenger");
    }
}