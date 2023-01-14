#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class IWindow;
    class Renderer_Vulkan;

    class Swapchain_Vulkan
    {
    public:

        static std::unique_ptr<Swapchain_Vulkan> create(
            const Renderer_Vulkan& renderer, 
            const IWindow& window, 
            const vk::PhysicalDevice& physical_device,
            const vk::Device& device);

        Swapchain_Vulkan(const Swapchain_Vulkan&) = delete;
        Swapchain_Vulkan(Swapchain_Vulkan&&) = delete;
        ~Swapchain_Vulkan();

        const vk::SwapchainKHR&       handle()       const { return m_swapchain_handle; }
        const std::vector<vk::Image>& images()       const { return m_images;           }
        const vk::Format&             image_format() const { return m_image_format;     }
        const vk::Extent2D&           extent()       const { return m_extent;           }
        vk::Viewport                  viewport()     const;
        vk::Rect2D                    scissor()      const;
    
    private:

        Swapchain_Vulkan(
            const vk::SwapchainKHR& swapchain_handle, 
            const std::vector<vk::Image>& images, 
            const vk::Format& image_format, 
            const vk::Extent2D& extent,
            const vk::Device& device);

        static vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats);
        static vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes);
        static vk::Extent2D choose_swap_extent(const IWindow& window, const vk::SurfaceCapabilitiesKHR& capabilities);

    private:

        vk::SwapchainKHR       m_swapchain_handle = nullptr;
        std::vector<vk::Image> m_images;
        vk::Format             m_image_format;
        vk::Extent2D           m_extent;

        vk::Device             m_device = nullptr;

    };
}