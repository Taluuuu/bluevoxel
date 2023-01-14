#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Renderer_Vulkan;

    class DebugMessenger_Vulkan
    {
    public:

        DebugMessenger_Vulkan(const DebugMessenger_Vulkan&) = delete;
        DebugMessenger_Vulkan(DebugMessenger_Vulkan&&) = delete;
        ~DebugMessenger_Vulkan();

        static std::unique_ptr<DebugMessenger_Vulkan> create(const Renderer_Vulkan& renderer);

        static void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& create_info);

    private:

        DebugMessenger_Vulkan(const vk::DebugUtilsMessengerEXT& messenger_handle, const Renderer_Vulkan& renderer);

    private:

        vk::DebugUtilsMessengerEXT m_messenger_handle = nullptr;

        const Renderer_Vulkan* const m_renderer = nullptr;

    };
}