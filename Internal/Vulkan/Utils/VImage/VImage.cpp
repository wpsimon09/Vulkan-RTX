//
// Created by wpsimon09 on 12/10/24.
//

#include "VImage.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

VulkanUtils::VImage::VImage(const VulkanCore::VDevice &device, vk::Image image, int widht, int height,
                            uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags aspecFlags):
    m_device(device), m_image(image) {

    m_width = widht;
    m_height = height;
    m_mipLevels = mipLevels;
    m_format = format;
    m_aspectFlags = aspecFlags;

    isSwapChainImage = true;

    GenerateImageView();
}

void VulkanUtils::VImage::Destroy() {
    m_device.GetDevice().destroyImageView(m_imageView);
    Utils::Logger::LogInfoVerboseOnly("Deleted image and its image view");
}

void VulkanUtils::VImage::GenerateImageView() {
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image = m_image;
    createInfo.format = m_format;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.subresourceRange.aspectMask = m_aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = m_mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    m_imageView = m_device.GetDevice().createImageView(createInfo);
    assert(m_imageView);
    Utils::Logger::LogInfoVerboseOnly("2D Image view created" );

}
