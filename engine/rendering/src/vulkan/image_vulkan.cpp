#include "image_vulkan.h"

#include "core/log.h"

namespace engine
{
    std::optional<Image_Vulkan> Image_Vulkan::create(const vk::Image& image, const vk::Format& format, const vk::Device& device)
    {
        assert(image);
        assert(device);

        auto image_view = create_image_view(image, format, device);
        if (!image_view)
            return std::nullopt;

        return Image_Vulkan(image, image_view, format, device);
    }

    std::vector<Image_Vulkan> Image_Vulkan::create(const std::vector<vk::Image>& image_handles, const vk::Format& format, const vk::Device& device)
    {
        std::vector<Image_Vulkan> result;
        result.reserve(image_handles.size());

        for (const auto& handle : image_handles)
        {
            auto image = create(handle, format, device);
            if (!image.has_value())
                return {};

            result.push_back(*image);
        }

        return result;
    }

    Image_Vulkan::Image_Vulkan(Image_Vulkan&& other)
    {
        m_image      = other.m_image;
        m_image_view = other.m_image_view;
        m_device     = other.m_device;

        other.m_image = nullptr;
        other.m_image_view = nullptr;
    }

    Image_Vulkan::~Image_Vulkan()
    {
        if (m_image)
            m_device.destroyImage(m_image);

        if (m_image_view)
            m_device.destroyImageView(m_image_view);
    }

    Image_Vulkan::Image_Vulkan(const vk::Image& image, const vk::ImageView& image_view, const vk::Format& format, const vk::Device& device)
        : m_image(image)
        , m_image_view(image_view)
        , m_format(format)
        , m_device(device)
    {
        
    }

    vk::ImageView Image_Vulkan::create_image_view(const vk::Image& image, const vk::Format& format, const vk::Device& device)
    {
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
            vk::ImageViewCreateInfo create_info({},
                image,
                vk::ImageViewType::e2D,
                format,
                components,
                subresource_range
            );

            return device.createImageView(create_info);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to create Vulkan image view: {}", e.what());
            return nullptr;
        }
    }
}