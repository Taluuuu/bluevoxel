#include "swapchain_vulkan.h"

#include "core/log.h"
#include "renderer_vulkan.h"
#include "windowing/window.h"

namespace engine
{
    std::unique_ptr<Swapchain_Vulkan> Swapchain_Vulkan::create(
        const Renderer_Vulkan& renderer, 
        const IWindow& window, 
        const vk::PhysicalDevice& physical_device,
        const vk::Device& device)
    {
        auto swapchain_support = renderer.query_swapchain_support(physical_device);

        auto surface_format = choose_swap_surface_format(swapchain_support.formats);
        auto present_mode = choose_swap_present_mode(swapchain_support.present_modes);
        auto extent = choose_swap_extent(window, swapchain_support.capabilities);

        u32 image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 && 
            image_count > swapchain_support.capabilities.maxImageCount)
        {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        // The vk::ImageUsageFlagBits parameter would be different if I need to draw
        // to a separate image first to do post processing
        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/swapchain
        vk::SwapchainCreateInfoKHR create_info({},
            renderer.surface(), 
            image_count, 
            surface_format.format, 
            surface_format.colorSpace, 
            extent, 
            1, 
            vk::ImageUsageFlagBits::eColorAttachment, 
            vk::SharingMode::eExclusive,
            0, nullptr, 
            swapchain_support.capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            present_mode,
            true,
            nullptr
        );

        auto indices = renderer.find_queue_families(physical_device);
        u32 queue_family_indices[] = 
        {
            indices.graphics_family.value(), 
            indices.present_family.value()
        };

        // Exclusive sharing mode offers best performance
        if (indices.graphics_family != indices.present_family)
        {
            create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        }

        try
        {
            auto swapchain_handle = device.createSwapchainKHR(create_info);
            return std::unique_ptr<Swapchain_Vulkan>(new Swapchain_Vulkan(
                swapchain_handle,
                device.getSwapchainImagesKHR(swapchain_handle),
                surface_format.format,
                extent,
                device));
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create Vulkan swapchain: {}", e.what());
            return nullptr;
        }
    }

    Swapchain_Vulkan::~Swapchain_Vulkan()
    {
        m_device.destroySwapchainKHR(m_swapchain_handle);
    }

    vk::Viewport Swapchain_Vulkan::viewport() const
    {
        return vk::Viewport(
            0.0f, 0.0f,
            static_cast<f32>(m_extent.width), static_cast<f32>(m_extent.height),
            0.0f, 1.0f
        );
    }

    vk::Rect2D Swapchain_Vulkan::scissor() const
    {
        return vk::Rect2D(
            { 0, 0 },
            m_extent
        );
    }

    Swapchain_Vulkan::Swapchain_Vulkan(const vk::SwapchainKHR& swapchain_handle, const std::vector<vk::Image>& images, const vk::Format& image_format, const vk::Extent2D& extent, const vk::Device& device)
        : m_swapchain_handle(swapchain_handle)
        , m_images(images)
        , m_image_format(image_format)
        , m_extent(extent)
        , m_device(device)
    {}

    vk::SurfaceFormatKHR Swapchain_Vulkan::choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats)
    {
        for (const auto& format : available_formats)
        {
            if (format.format == vk::Format::eB8G8R8A8Srgb && 
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return format;
            }
        }

        assert(!available_formats.empty());
        return available_formats[0];
    }

    vk::PresentModeKHR Swapchain_Vulkan::choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes)
    {
        // Present modes : 
        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/swapchain
        
        for (const auto& present_mode : available_present_modes)
        {
            if (present_mode == vk::PresentModeKHR::eMailbox)
                return present_mode;
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Swapchain_Vulkan::choose_swap_extent(const IWindow& window, const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
            return capabilities.currentExtent;

        auto window_size = window.framebuffer_size();
        vk::Extent2D actual_extent(window_size.x, window_size.y);
        
        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}