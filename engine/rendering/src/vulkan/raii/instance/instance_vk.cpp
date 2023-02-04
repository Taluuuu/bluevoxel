#include "instance_vk.h"

#include "core/log.h"
#include "debug_messenger_vk.h"
#include "surface_vk.h"

namespace engine
{
    std::shared_ptr<InstanceVk> engine::InstanceVk::create(const VkInstanceCreateInfo& create_info)
    {
        VkInstance instance;
        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
        {
            log::error("Failed to create Vulkan instance.");
            return nullptr;
        }

        return std::shared_ptr<InstanceVk>(new InstanceVk(instance));
    }

    InstanceVk::~InstanceVk()
    {
        vkDestroyInstance(m_instance, nullptr);
    }

    DebugMessengerVk* InstanceVk::create_debug_messenger()
    {
        return DebugMessengerVk::create(shared_from_this());
    }

    SurfaceVk* InstanceVk::create_surface(const IWindow& window)
    {
        return SurfaceVk::create(shared_from_this(), window);
    }

    InstanceVk::InstanceVk(VkInstance instance)
        : m_instance(instance)
    {}
}