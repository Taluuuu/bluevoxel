#include "image_array_vulkan.h"

#include "image_vulkan.h"

namespace engine
{
    std::optional<ImageArray_Vulkan> ImageArray_Vulkan::create(const std::vector<vk::Image> &images_vk, const vk::Format &format, const vk::Device &device, bool destroy_handles)
    {
        std::vector<std::shared_ptr<Image_Vulkan>> images(images_vk.size(), nullptr);
        for (size_t i = 0; i < images_vk.size(); i++)
        {
            auto image = Image_Vulkan::create(images_vk[i], format, device, destroy_handles);
            if (!image)
                return std::nullopt;

            images[i] = std::move(image);
        }

        return ImageArray_Vulkan(images);
    }

    ImageArray_Vulkan::ImageArray_Vulkan(ImageArray_Vulkan&& other)
    {
        m_images = other.m_images;
        other.m_images.clear();
    }

    std::vector<ImageView_Vulkan> ImageArray_Vulkan::create_image_views()
    {
        std::vector<ImageView_Vulkan> image_views;
        image_views.reserve(m_images.size());

        for (const auto& image : m_images)
        {
            assert(image);
            auto image_view = image->create_image_view();
            if (!image_view.has_value())
                return {};

            image_views.push_back(std::move(*image_view));
        }

        return image_views;
    }

    ImageArray_Vulkan::ImageArray_Vulkan(const std::vector<std::shared_ptr<Image_Vulkan>>& images)
        : m_images(images)
    {}
}