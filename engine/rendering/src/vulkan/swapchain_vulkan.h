#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class ImageView_Vulkan;
    class IWindow;
    class Renderer_Vulkan;

    class Swapchain_Vulkan
    {
    public:

        static Swapchain_Vulkan* create(
            const Renderer_Vulkan& renderer, 
            const IWindow& window);

        Swapchain_Vulkan(const Swapchain_Vulkan&) = delete;
        Swapchain_Vulkan(Swapchain_Vulkan&&) = delete;
        ~Swapchain_Vulkan();

        void recreate();

        const vk::SwapchainKHR&             handle()       const { return m_swapchain;    }
        const std::vector<vk::Image>&       images()       const { return m_images;       }
        const vk::Format&                   image_format() const { return m_image_format; }
        const vk::Extent2D&                 extent()       const { return m_extent;       }
        const vk::RenderPass&               render_pass()  const { return m_render_pass;  }
        const std::vector<vk::Framebuffer>& framebuffers() const { return m_framebuffers; }
        vk::Viewport                        viewport()     const;
        vk::Rect2D                          scissor()      const;
    
    private:

        Swapchain_Vulkan(
            const vk::SwapchainKHR&              swapchain, 
            const vk::RenderPass&                render_pass,
            const vk::Extent2D&                  extent,
            const vk::Format&                    image_format, 
            const std::vector<vk::Image>&        images,
            std::vector<ImageView_Vulkan>& image_views,
            const std::vector<vk::Framebuffer>&  framebuffers,
            const Renderer_Vulkan&               renderer);

        static vk::SwapchainKHR             create_swapchain(const Renderer_Vulkan& renderer, const IWindow& window, const vk::SurfaceFormatKHR& surface_format, const vk::PresentModeKHR& present_mode, const vk::Extent2D& extent, const vk::SurfaceCapabilitiesKHR& capabilities);
        static std::vector<vk::ImageView>   create_image_views(const Renderer_Vulkan& renderer, const vk::SwapchainKHR& swapchain, const vk::Format& image_format, const std::vector<vk::Image>& images);
        static vk::RenderPass               create_render_pass(const vk::Device& device, const vk::Format& image_format);
        static std::vector<vk::Framebuffer> create_framebuffers(const std::vector<ImageView_Vulkan>& image_views, const vk::RenderPass& render_pass, const vk::Extent2D& extent, const vk::Device& device);

        static vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats);
        static vk::PresentModeKHR   choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes);
        static vk::Extent2D         choose_swap_extent(const IWindow& window, const vk::SurfaceCapabilitiesKHR& capabilities);

    private:

        vk::SwapchainKHR              m_swapchain = nullptr;
        vk::RenderPass                m_render_pass = nullptr;

        vk::Extent2D                  m_extent;

        vk::Format                    m_image_format;
        std::vector<vk::Image>        m_images;
        std::vector<ImageView_Vulkan> m_image_views;
        std::vector<vk::Framebuffer>  m_framebuffers;

        const Renderer_Vulkan* const  m_renderer = nullptr;

    };
}