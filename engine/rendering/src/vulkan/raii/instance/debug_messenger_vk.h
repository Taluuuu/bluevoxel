#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace engine
{
    class InstanceVk;

    class DebugMessengerVk
    {
    public:

        [[nodiscard]] static DebugMessengerVk* create(const std::shared_ptr<InstanceVk>& instance);

        DebugMessengerVk(const DebugMessengerVk&) = delete;
        DebugMessengerVk(DebugMessengerVk&& other) = delete;
        ~DebugMessengerVk();
    
    private:

        DebugMessengerVk(VkDebugUtilsMessengerEXT messenger, const std::shared_ptr<InstanceVk>& instance);

    private:

        VkDebugUtilsMessengerEXT m_messenger = nullptr;

        const std::shared_ptr<InstanceVk> m_instance = nullptr;

    };
}