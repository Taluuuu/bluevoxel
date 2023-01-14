#include "debug_messenger_vulkan.h"

#include "core/log.h"
#include "renderer_vulkan.h"

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
    engine::log::warn("Vulkan validation error: {}", callback_data->pMessage);
    return VK_FALSE;
}

namespace engine
{
    DebugMessenger_Vulkan::~DebugMessenger_Vulkan()
    {
        if (m_messenger_handle)
            m_renderer->instance().destroyDebugUtilsMessengerEXT(m_messenger_handle);
    }

    std::unique_ptr<DebugMessenger_Vulkan> DebugMessenger_Vulkan::create(const Renderer_Vulkan& renderer)
    {
        auto instance = renderer.instance();

        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        if (!pfnVkCreateDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr( "vkDestroyDebugUtilsMessengerEXT"));
        if (!pfnVkDestroyDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

        vk::DebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);

        try
        {
            auto messenger_handle = instance.createDebugUtilsMessengerEXT(create_info);
            return std::unique_ptr<DebugMessenger_Vulkan>(new DebugMessenger_Vulkan(messenger_handle, renderer));
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create Vulkan debug messenger: {}", e.what());
        }
        
        return nullptr;
    }

    void DebugMessenger_Vulkan::populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& create_info)
    {
        const vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags = 
            // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        const vk::DebugUtilsMessageTypeFlagsEXT message_type_flags = 
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

        create_info = vk::DebugUtilsMessengerCreateInfoEXT({},
            message_severity_flags,
            message_type_flags,
            debug_callback
        );
    }

    DebugMessenger_Vulkan::DebugMessenger_Vulkan(const vk::DebugUtilsMessengerEXT& messenger_handle, const Renderer_Vulkan& renderer)
        : m_messenger_handle(messenger_handle)
        , m_renderer(&renderer)
    {}
}