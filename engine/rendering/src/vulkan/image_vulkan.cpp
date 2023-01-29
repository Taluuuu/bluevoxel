#include "image_vulkan.h"

#include "core/log.h"

namespace engine
{
    std::shared_ptr<Image_Vulkan> Image_Vulkan::create(
        const vk::Image& image, 
        const vk::Format& format, 
        const vk::Device& device,
        bool destroy_handle)
    {
        assert(image);
        assert(device);

        return std::shared_ptr<Image_Vulkan>(
            new Image_Vulkan(image, format, device, destroy_handle));
    }

    Image_Vulkan::Image_Vulkan(Image_Vulkan&& other)
    {
        m_image  = other.m_image;
        m_format = other.m_format;
        m_device = other.m_device;

        other.m_image = nullptr;
    }

    Image_Vulkan::~Image_Vulkan()
    {
        if (m_image && m_destroy_handle)
            m_device.destroyImage(m_image);
    }

    std::optional<ImageView_Vulkan> Image_Vulkan::create_image_view() const
    {
        return ImageView_Vulkan::create(shared_from_this());
    }

    Image_Vulkan::Image_Vulkan(
        const vk::Image& image, 
        const vk::Format& format, 
        const vk::Device& device, 
        bool destroy_handle)
        : m_image(image)
        , m_format(format)
        , m_device(device)
        , m_destroy_handle(destroy_handle)
    {}
}