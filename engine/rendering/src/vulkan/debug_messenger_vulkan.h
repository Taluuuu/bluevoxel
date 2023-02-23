#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace engine
{
    class Renderer_Vulkan;

    class DebugMessenger_Vulkan
    {
    public:

        static DebugMessenger_Vulkan* create(const Renderer_Vulkan& renderer);

        DebugMessenger_Vulkan(const DebugMessenger_Vulkan&) = delete;
        DebugMessenger_Vulkan(DebugMessenger_Vulkan&&) = delete;
        ~DebugMessenger_Vulkan();

        static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);

    private:

        DebugMessenger_Vulkan(VkDebugUtilsMessengerEXT messenger, const Renderer_Vulkan& renderer);

    private:

        VkDebugUtilsMessengerEXT m_messenger = nullptr;

        const Renderer_Vulkan* const m_renderer = nullptr;

    };
}