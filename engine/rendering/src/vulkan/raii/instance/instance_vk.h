#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace engine
{
    class DebugMessengerVk;
    class SurfaceVk;
    class IWindow;

    class InstanceVk : std::enable_shared_from_this<InstanceVk>
    {
    public:

        [[nodiscard]] static std::shared_ptr<InstanceVk> create(const VkInstanceCreateInfo& create_info);

        InstanceVk(const InstanceVk&) = delete;
        InstanceVk(InstanceVk&& other) = delete;
        ~InstanceVk();

        [[nodiscard]] VkInstance handle() const { return m_instance; }

        [[nodiscard]] DebugMessengerVk* create_debug_messenger();
        [[nodiscard]] SurfaceVk*        create_surface(const IWindow& window);

    private:

        InstanceVk(VkInstance instance);

    private:

        VkInstance m_instance = nullptr;

    };
}