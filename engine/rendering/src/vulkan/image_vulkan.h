#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Image_Vulkan
    {
    public:

        static std::optional<Image_Vulkan> create(const vk::Image& image, const vk::Format& format, const vk::Device& device);
        static std::vector<Image_Vulkan>   create(const std::vector<vk::Image>& images, const vk::Format& format, const vk::Device& device);

        Image_Vulkan(const Image_Vulkan&) = delete;
        Image_Vulkan(Image_Vulkan&& other);
        ~Image_Vulkan();

        const vk::Image&     handle() const { return m_image;      }
        const vk::ImageView& view()   const { return m_image_view; }
        const vk::Format&    format() const { return m_format;     }

    private:

        Image_Vulkan(
            const vk::Image&     image, 
            const vk::ImageView& image_view, 
            const vk::Format&    format,
            const vk::Device&    device);

        static vk::ImageView create_image_view(const vk::Image& image, const vk::Format& format, const vk::Device& device);
    
    private:

        vk::Image     m_image        = nullptr;
        vk::ImageView m_image_view   = nullptr;
        vk::Format    m_format;

        vk::Device    m_device       = nullptr;

    };
}