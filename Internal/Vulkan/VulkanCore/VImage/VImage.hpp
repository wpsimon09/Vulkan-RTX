//
// Created by wpsimon09 on 12/10/24.
//

#ifndef VIMAGE_HPP
#define VIMAGE_HPP
#include <VMA/vk_mem_alloc.h>
#include <bits/fs_path.h>
#include <vulkan/vulkan.hpp>
#include "Vulkan/VulkanCore/VObject.hpp"


namespace VulkanCore
{
    class VDevice;
}

namespace VulkanCore
{
    class VImage:public VulkanCore::VObject {
    public:
        // creates image and iamge views from existing image, mostly used for retrieving SwapChain images
        VImage(const VulkanCore::VDevice& device,vk::Image image, int widht, int height, uint32_t mipLevels = 1, vk::Format format = vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);

        VImage(const VulkanCore::VDevice& device,std::string path,uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags aspecFlags);

        void Destroy() override;

        ~VImage() = default;
    private:
        void GenerateImage(std::string path );

        void GenerateImageView();
    private:
        const VulkanCore::VDevice& m_device;
        vk::Image m_imageVK;
        VkImage m_imageVMA;
        vk::ImageView m_imageView;
        VmaAllocation m_imageAllocation;
        uint32_t m_mipLevels;
        vk::Format m_format;
        vk::ImageAspectFlags m_aspectFlags;
        vk::DeviceSize m_imageSize;
        bool isSwapChainImage = false;
        int m_width, m_height;

    public:
        const bool& GetIsSwapChainImage() const {return isSwapChainImage;};

        [[nodiscard]] const vk::Image & GetImage() const {
            return m_imageVK;
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
