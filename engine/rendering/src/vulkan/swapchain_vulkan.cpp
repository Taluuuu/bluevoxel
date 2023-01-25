#include "swapchain_vulkan.h"

#include "device_vulkan.h"
#include "renderer_vulkan.h"
#include "surface_vulkan.h"

#include "core/log.h"
#include "windowing/window.h"

namespace engine
{
    Swapchain_Vulkan* Swapchain_Vulkan::create(
        const Renderer_Vulkan& renderer, 
        const IWindow& window)
    {
        auto device = renderer.device().handle();

        vk::Format image_format;
        auto swapchain = create_swapchain(renderer, window, image_format);
        if (!swapchain)
            return nullptr;

        auto images = device.getSwapchainImagesKHR(swapchain);
        auto image_views = create_image_views(renderer, swapchain, image_format, images);
        if (image_views.empty())
        {
            device.destroySwapchainKHR(swapchain);
            return nullptr;
        }

        auto render_pass = create_render_pass(device, image_format);
        if (!render_pass)
        {
            for (const auto& image_view : image_views)
                device.destroyImageView(image_view);
            
            device.destroySwapchainKHR(swapchain);
            return nullptr;
        }

        try
        {
            // Sort le extent
            return new Swapchain_Vulkan(
                swapchain,
                render_pass,
                extent;
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

    void Swapchain_Vulkan::recreate()
    {
        
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

    Swapchain_Vulkan::Swapchain_Vulkan(
        const vk::SwapchainKHR&             swapchain, 
        const vk::RenderPass&               render_pass,
        const vk::Extent2D&                 extent,
        const vk::Format&                   image_format, 
        const std::vector<vk::Image>&       images, 
        const std::vector<vk::ImageView>&   image_views,
        const std::vector<vk::Framebuffer>& framebuffers,
        const Renderer_Vulkan&              renderer)
        : m_swapchain(swapchain)
        , m_render_pass(render_pass)
        , m_extent(extent)
        , m_image_format(image_format)
        , m_images(images)
        , m_image_views(image_views)
        , m_framebuffers(framebuffers)
        , m_renderer(&renderer)
    {}

    vk::SwapchainKHR Swapchain_Vulkan::create_swapchain(const Renderer_Vulkan& renderer, const IWindow& window, vk::Format& out_image_format)
    {
        const auto& physical_device = renderer.device().physical_device_handle();
        const auto& logical_device = renderer.device().handle();

        auto swapchain_support = renderer.surface().query_swapchain_support(physical_device);

        auto surface_format = choose_swap_surface_format(swapchain_support.formats);
        auto present_mode = choose_swap_present_mode(swapchain_support.present_modes);
        auto extent = choose_swap_extent(window, swapchain_support.capabilities);
        out_image_format = surface_format.format;

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
            renderer.surface().handle(), 
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
            return logical_device.createSwapchainKHR(create_info);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create Vulkan Swapchain: {}", e.what());
            return nullptr;
        }
    }

    std::vector<vk::ImageView> Swapchain_Vulkan::create_image_views(const Renderer_Vulkan& renderer, const vk::SwapchainKHR& swapchain, const vk::Format& image_format, const std::vector<vk::Image>& images)
    {
        const auto& logical_device = renderer.device().handle();

        auto swapchain_images = logical_device.getSwapchainImagesKHR(swapchain);
        std::vector<vk::ImageView> image_views(swapchain_images.size(), nullptr);

        vk::ComponentMapping components(
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
        );

        vk::ImageSubresourceRange subresource_range(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        try
        {
            for (size_t i = 0; i < swapchain_images.size(); i++)
            {
                vk::ImageViewCreateInfo create_info({},
                    swapchain_images[i],
                    vk::ImageViewType::e2D,
                    image_format,
                    components,
                    subresource_range
                );

                image_views [i] = logical_device.createImageView(create_info);
            }

            return image_views;
        }
        catch (const std::exception& e)
        {
            // Destroy created swapchain image views
            for (const auto& image_view : image_views)
            {
                if (image_view)
                    logical_device.destroyImageView(image_view);
            }

            log::error("Failed to create Vulkan Swapchain images: {}", e.what());
            return {};
        }
    }

    vk::RenderPass Swapchain_Vulkan::create_render_pass(const vk::Device& device, const vk::Format& image_format)
    {
        vk::AttachmentDescription color_attachment({},
            image_format,
            vk::SampleCountFlagBits::e1, // Should match the format of the swapchain images
            vk::AttachmentLoadOp::eClear, // Clear screen to black
            vk::AttachmentStoreOp::eStore, // Store rendered content
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
        );

        vk::AttachmentReference color_attachment_ref(
            0, vk::ImageLayout::eColorAttachmentOptimal
        );

        vk::SubpassDescription subpass({},
            vk::PipelineBindPoint::eGraphics,
            0, nullptr,
            // The index of the attachment in this array is directly referenced from the fragment 
            // shader with the layout(location = 0)
            1, &color_attachment_ref, 
            nullptr,
            nullptr,
            0, nullptr
        );

        vk::SubpassDependency dependency(
            VK_SUBPASS_EXTERNAL, 0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput, 
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite
        );

        vk::RenderPassCreateInfo render_pass_create_info({},
            1, &color_attachment,
            1, &subpass,
            1, &dependency
        );

        try
        {
            return device.createRenderPass(render_pass_create_info);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to craete render pass: {}", e.what());
            return nullptr;
        }
    }

    vk::SurfaceFormatKHR Swapchain_Vulkan::choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR> &available_formats)
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