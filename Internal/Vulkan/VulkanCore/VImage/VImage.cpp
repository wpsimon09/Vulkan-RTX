//
// Created by wpsimon09 on 12/10/24.
//

#include "VImage.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "stb_image/stb_image.h"

VulkanCore::VImage::VImage(const VulkanCore::VDevice &device, vk::Image image, int widht, int height,
                            uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags aspecFlags):
    m_device(device), m_imageVK(image) {

    m_width = widht;
    m_height = height;
    m_mipLevels = mipLevels;
    m_format = format;
    m_aspectFlags = aspecFlags;

    isSwapChainImage = true;

    GenerateImageView();
}

VulkanCore::VImage::VImage(const VulkanCore::VDevice &device, std::string path,uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags aspecFlags): m_device(device) {
    m_mipLevels = mipLevels;
    m_format = format;
    m_aspectFlags = aspecFlags;

    isSwapChainImage = false;
    GenerateImage(path);
}

void VulkanCore::VImage::Destroy() {
    m_device.GetDevice().destroyImageView(m_imageView);
    if(!isSwapChainImage) {
        m_device.GetDevice().destroyImage(m_imageVK);
    }
    Utils::Logger::LogInfoVerboseOnly("Deleted image and its image view");
}

void VulkanCore::VImage::GenerateImage(std::string path) {
    Utils::Logger::LogInfoVerboseOnly("Generating image...");
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    m_imageSize = texWidth * texHeight * 4;

    if(!pixels) {
        Utils::Logger::LogError("Failed to generate texture at path: \t" + path);
        return;
    }
    else {
        Utils::Logger::LogInfoVerboseOnly("Image read successfully !");
    }

    // copy pixel data to the staging buffer
    Utils::Logger::LogInfoVerboseOnly("Copying image data to staging buffer");
    auto stagingBuffer = VulkanCore::VBuffer(m_device);
    stagingBuffer.CreateStagingBuffer(m_imageSize);
    auto mappedStagingBuffer = stagingBuffer.MapStagingBuffer();
    memcpy(mappedStagingBuffer, pixels, static_cast<size_t>(m_imageSize));
    Utils::Logger::LogInfoVerboseOnly("Image data copied");

    //create vulkan represetnation of the image
    VkImageCreateInfo imageInfo = {};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = m_mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = static_cast<VkFormat>(m_format);
    imageInfo.tiling = static_cast<VkImageTiling>(vk::ImageTiling::eOptimal);
    imageInfo.initialLayout = static_cast<VkImageLayout>(vk::ImageLayout::eUndefined);
    imageInfo.usage = static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
    imageInfo.sharingMode  = static_cast<VkSharingMode>(vk::SharingMode::eExclusive);
    imageInfo.samples  = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);

    m_imageVK = m_device.GetDevice().createImage(imageInfo);

    // create vma allocation
    VmaAllocationCreateInfo imageAllocationInfo = {};
    imageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    imageAllocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    vmaCreateImage(m_device.GetAllocator(), &imageInfo, &imageAllocationInfo, &m_imageVMA, &m_imageAllocation, nullptr);
}

void VulkanCore::VImage::GenerateImageView() {
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image = m_imageVK;
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
