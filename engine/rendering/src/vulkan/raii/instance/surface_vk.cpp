#include "surface_vk.h"

#include "core/log.h"
#include "instance_vk.h"
#include "windowing/window.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace engine
{
    SurfaceVk* SurfaceVk::create(const std::shared_ptr<InstanceVk>& instance, const IWindow& window)
    {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(
            instance->handle(), 
            reinterpret_cast<GLFWwindow*>(window.handle()), 
            nullptr, 
            &surface) != VK_SUCCESS)
        {
            log::error("Failed to create Vulkan GLFW window surface.");
            return nullptr;
        }

        return new SurfaceVk(instance, surface);
    }

    SurfaceVk::~SurfaceVk()
    {
        vkDestroySurfaceKHR(m_instance->handle(), m_surface, nullptr);
    }

    SurfaceVk::SurfaceVk(const std::shared_ptr<InstanceVk>& instance, VkSurfaceKHR surface)
        : m_instance(instance)
        , m_surface(surface)
    {}
}