#pragma once

#include "core/types.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace engine
{
    class Renderer_Vulkan;
    class IWindow;

    class Surface_Vulkan
    {
    public:

        static Surface_Vulkan* create(const IWindow& window, const Renderer_Vulkan& renderer);

        Surface_Vulkan(const Surface_Vulkan&) = delete;
        Surface_Vulkan(Surface_Vulkan&&) = delete;
        ~Surface_Vulkan();

        vk::SurfaceKHR handle() const { return m_surface_handle; }

        bool surface_support(const vk::PhysicalDevice& device, u32 queue_family_index) const;

        struct SwapchainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> present_modes;

            bool is_adequate() const
            {
                return
                    !formats.empty() &&
                    !present_modes.empty();
            }
        };

        SwapchainSupportDetails query_swapchain_support(const vk::PhysicalDevice& device) const;

    private:

        Surface_Vulkan(const vk::SurfaceKHR& surface, const Renderer_Vulkan& renderer);

    private:

        vk::SurfaceKHR m_surface_handle = nullptr;

        const Renderer_Vulkan* const m_renderer = nullptr;
    
    };
}