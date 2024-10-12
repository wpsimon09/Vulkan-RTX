//
// Created by wpsimon09 on 12/10/24.
//

#ifndef VIMAGE_HPP
#define VIMAGE_HPP
#include <vulkan/vulkan.hpp>
#include "Vulkan/VulkanCore/VObject.hpp"


namespace VulkanCore
{
    class VDevice;
}

namespace VulkanUtils
{
    class VImage:VulkanCore::VObject {
    public:
        VImage(const VulkanCore::VDevice& device,vk::Image &image, int widht, int height, uint32_t mipLevels = 1, vk::Format format = vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);

        void Destroy() override;

        ~VImage();
    private:
        //TODO: later implement this or use the VMA lybrary to allocate images and generate their image views ;
        void GenerateImage() {};
        void GenerateImageView();
    private:
        const VulkanCore::VDevice& m_device;
        vk::Image m_image;
        vk::ImageView m_imageView;
        uint32_t m_mipLevels;
        vk::Format m_format;
        vk::ImageAspectFlags m_aspectFlags;
        int m_width, m_height;

    public:
        [[nodiscard]] const vk::Image & GetImage() const {
            return m_image;
        }

        [[nodiscard]] const vk::ImageView & GetImageView() const {
            return m_imageView;
        }

        [[nodiscard]] uint32_t GetMipLevelels() const {
            return m_mipLevels;
        }

        [[nodiscard]] vk::Format GetFormat() const {
            return m_format;
        }

        [[nodiscard]] const vk::ImageAspectFlags & GetAspectFlag() const {
            return m_aspectFlags;
        }

        [[nodiscard]] int GetWidth() const {
            return m_width;
        }

        [[nodiscard]] int GetHeight() const {
            return m_height;
        }
    };
}


#endif //VIMAGE_HPP
