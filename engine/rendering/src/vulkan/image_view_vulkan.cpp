#include "image_view_vulkan.h"

#include "core/log.h"
#include "image_vulkan.h"

namespace engine
{
    std::optional<ImageView_Vulkan> ImageView_Vulkan::create(
        const std::shared_ptr<const Image_Vulkan>& image)
    {
        assert(image);

        vk::ComponentMapping components(
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity);

        vk::ImageSubresourceRange subresource_range(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        try
        {
            vk::ImageViewCreateInfo create_info({},
                image->handle(),
                vk::ImageViewType::e2D,
                image->format(),
                components,
                subresource_range);

            return ImageView_Vulkan(
                image->device().createImageView(create_info),
                image);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create Vulkan image view: {}", e.what());
            return std::nullopt;
        }
    }

    ImageView_Vulkan::~ImageView_Vulkan()
    {
        if (m_image_view)
            m_device.destroyImageView(m_image_view);
    }

    ImageView_Vulkan::ImageView_Vulkan(ImageView_Vulkan&& other)
    {
        m_image = other.m_image;
        m_image_view = other.m_image_view;
        m_device = other.m_device;

        other.m_image = nullptr;
        other.m_image_view = nullptr;
    }

    ImageView_Vulkan::ImageView_Vulkan(
        const vk::ImageView& image_view,
        const std::shared_ptr<const Image_Vulkan>& image)
        : m_image_view(image_view)
        , m_image(image)
        , m_device(image->device())
    {}
}