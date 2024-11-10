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
    m_transferCommandBuffer = std::make_unique<VCommandBuffer>(m_device, m_device.GetTransferCommandPool());

    // this command buffer will record all commands that are needed for image to be created and execute them all at once
    m_transferCommandBuffer->BeginRecording();

    GenerateImage(path);
    // make buffer layout best for transition data into
    TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal);
    CopyFromBufferToImage();
    //make buffer layout best for shader to read from
    TransitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    m_transferCommandBuffer->EndAndFlush(m_device.GetTransferQueue());
    m_stagingBufferWithPixelData->Destroy();
}

void VulkanCore::VImage::Destroy() {
    m_device.GetDevice().destroyImageView(m_imageView);
    if(!isSwapChainImage) {
        m_device.GetDevice().destroyImage(m_imageVK);
    }
    Utils::Logger::LogInfoVerboseOnly(  "Deleted image and its image view");
}

//------------------------------------------------------------
// ALLOCATE IMAGE , FILL WITH DATA, MAKE IT SHADER ACCESSIBLE
//------------------------------------------------------------
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
    m_stagingBufferWithPixelData = std::make_unique<VulkanCore::VBuffer>(m_device);
    m_stagingBufferWithPixelData->CreateStagingBuffer(m_imageSize);
    auto mappedStagingBuffer = m_stagingBufferWithPixelData->MapStagingBuffer();
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

    //m_imageVK = m_device.GetDevice().createImage(imageInfo);

    // create vma allocation
    VmaAllocationCreateInfo imageAllocationInfo = {};
    imageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    imageAllocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    vmaCreateImage(m_device.GetAllocator(), &imageInfo, &imageAllocationInfo, &m_imageVMA, &m_imageAllocation, nullptr);

    m_imageVK = m_imageVMA;
}

//--------------------------
// IMAGE LAYOUT TRANSITION
//--------------------------
void VulkanCore::VImage::TransitionImageLayout(vk::ImageLayout targetLayout) {
    Utils::Logger::LogInfoVerboseOnly("Transition image layout...");

    // record
    m_transferCommandBuffer->BeginRecording();

    vk::PipelineStageFlags srcStageFlags;
    vk::PipelineStageFlags dstStageFlags;

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_imageLayout;
    barrier.newLayout = targetLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex= vk::QueueFamilyIgnored;
    barrier.image = m_imageVK;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if(m_imageLayout==vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eTransferDstOptimal){
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(m_imageLayout == vk::ImageLayout::eTransferDstOptimal && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }

    assert(m_transferCommandBuffer->GetIsRecording() && "Command buffer that will transfer image layout is not recording !");
    m_transferCommandBuffer->GetCommandBuffer().pipelineBarrier(
      srcStageFlags, dstStageFlags,
          {},
          0, nullptr,
          0, nullptr,
          1, &barrier
      );

    m_imageLayout = targetLayout;
}


//-----------------------------
// STAGING BUFFER -> IMAGE CPY
//-----------------------------
void VulkanCore::VImage::CopyFromBufferToImage() {
    Utils::Logger::LogInfoVerboseOnly("Copying image from buffer to image...");
    vk::BufferImageCopy imageCopyInfo{
        0, 0,0,
        {   vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {0,0,0},
        {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 0}
    };

    assert(m_transferCommandBuffer->GetIsRecording() && "Command buffer that will copy buffer to image is not recording");
    m_transferCommandBuffer->GetCommandBuffer().copyBufferToImage(m_stagingBufferWithPixelData->GetStagingBuffer(), m_imageVK, vk::ImageLayout::eTransferDstOptimal,1, &imageCopyInfo);
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
