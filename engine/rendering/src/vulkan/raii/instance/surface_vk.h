#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace engine
{
    class InstanceVk;
    class IWindow;

    class SurfaceVk
    {
    public:

        [[nodiscard]] static SurfaceVk* create(const std::shared_ptr<InstanceVk>& instance, const IWindow& window);

        SurfaceVk(const SurfaceVk&) = delete;
        SurfaceVk(SurfaceVk&&) = delete;
        ~SurfaceVk();

    private:

        SurfaceVk(const std::shared_ptr<InstanceVk>& instance, VkSurfaceKHR surface);

    private:

        VkSurfaceKHR m_surface = nullptr;

        const std::shared_ptr<InstanceVk> m_instance = nullptr;

    };
}