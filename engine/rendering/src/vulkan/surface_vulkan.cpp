#include "surface_vulkan.h"

#include "device_vulkan.h"
#include "renderer_vulkan.h"

#include "core/log.h"
#include "windowing/window.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace engine
{
    Surface_Vulkan* Surface_Vulkan::create(const IWindow& window, const Renderer_Vulkan& renderer)
    {
        VkSurfaceKHR surface;
        auto result = glfwCreateWindowSurface(
            renderer.instance(),
            reinterpret_cast<GLFWwindow*>(window.handle()),
            nullptr,
            &surface);

        if (result != VK_SUCCESS)
        {
            log::error("Failed to create Vulkan GLFW window surface.");
            return nullptr;
        }

        return new Surface_Vulkan(surface, renderer);
    }

    Surface_Vulkan::~Surface_Vulkan()
    {
        assert(m_surface_handle);
        m_renderer->instance().destroySurfaceKHR(m_surface_handle);
    }

    bool Surface_Vulkan::surface_support(const vk::PhysicalDevice& device, u32 queue_family_index) const
    {
        return device.getSurfaceSupportKHR(queue_family_index, m_surface_handle);
    }

    Surface_Vulkan::SwapchainSupportDetails Surface_Vulkan::query_swapchain_support(const vk::PhysicalDevice& device) const
    {
        return SwapchainSupportDetails
        {
            .capabilities  = device.getSurfaceCapabilitiesKHR(m_surface_handle),
            .formats       = device.getSurfaceFormatsKHR(m_surface_handle),
            .present_modes = device.getSurfacePresentModesKHR(m_surface_handle)
        };
    }

    Surface_Vulkan::Surface_Vulkan(const vk::SurfaceKHR& surface, const Renderer_Vulkan& renderer)
        : m_surface_handle(surface)
        , m_renderer(&renderer)
    {}
}