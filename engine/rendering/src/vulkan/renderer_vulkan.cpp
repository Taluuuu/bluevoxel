#include "renderer_vulkan.h"

#include "core/game_info.h"
#include "core/types.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace engine
{
    static VkResult create_debug_utils_messenger_ext(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* create_info, 
        const VkAllocationCallbacks* allocator, 
        VkDebugUtilsMessengerEXT* debug_messenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, create_info, allocator, debug_messenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroy_debug_utils_messenger_ext(
        VkInstance instance, 
        VkDebugUtilsMessengerEXT debug_messenger, 
        const VkAllocationCallbacks* allocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debug_messenger, allocator);
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

    Renderer_Vulkan::Renderer_Vulkan(const GameInfo& game_info, const IWindow& window)
    {
        create_instance(game_info.game_name.data(), game_info.engine_name.data());
        setup_debug_messenger();
    }

    Renderer_Vulkan::~Renderer_Vulkan()
    {
        if (m_enable_validation_layers)
            destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, nullptr);

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
            VK_API_VERSION_1_3
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

        vk::DebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);
    }

    std::vector<const char *> Renderer_Vulkan::get_required_extensions() const
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
        u32 layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

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
}