#include "debug_messenger_vk.h"

#include <cassert>

#include "core/log.h"
#include "instance_vk.h"

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
    DebugMessengerVk* DebugMessengerVk::create(const std::shared_ptr<InstanceVk>& instance)
    {
        assert(instance);

        VkDebugUtilsMessengerCreateInfoEXT create_info
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = {},
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = nullptr
        };

        VkDebugUtilsMessengerEXT messenger;
        if (vkCreateDebugUtilsMessengerEXT(instance->handle(), &create_info, nullptr, &messenger) != VK_SUCCESS)
        {
            log::error("Failed to create Vulkan debug messenger.");
            return nullptr;
        }

        return new DebugMessengerVk(messenger, instance);
    }

    DebugMessengerVk::~DebugMessengerVk()
    {
        vkDestroyDebugUtilsMessengerEXT(m_instance->handle(), m_messenger, nullptr);
    }

    DebugMessengerVk::DebugMessengerVk(VkDebugUtilsMessengerEXT messenger, const std::shared_ptr<InstanceVk>& instance)
        : m_messenger(messenger)
        , m_instance(instance)
    {}
}