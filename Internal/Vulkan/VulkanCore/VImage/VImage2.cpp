//
// Created by wpsimon09 on 12/03/25.
//

#include "VImage2.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace VulkanCore {
    VImage2::VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& info):
        m_device(device), m_imageInfo(info)
    {
        AllocateImage();
        GenerateImageView();


    }

    VImage2::VImage2(const VulkanCore::VDevice& device,const  VImage2CreateInfo& info, vk::Image swapChainImage):
        m_device(device), m_imageInfo(info), m_imageVK(swapChainImage), m_imageFlags{}
    {
        AllocateImage();
        GenerateImageView();
    }

    VImage2::VImage2(const VulkanCore::VDevice& device, VulkanStructs::ImageData<uint32_t>& imageData):
        m_device(device),m_imageInfo{}, m_imageFlags{}
    {
        AllocateImage();
        GenerateImageView();
    }

    void VImage2::Resize(uint32_t newWidth, uint32_t newHeight, vk::CommandBuffer& cmdBuffer)
    {
        AllocateImage();
        GenerateImageView();
    }

    void VImage2::Destroy()
    {
        VObject::Destroy();
    }

    void VImage2::AllocateImage()
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_imageInfo.width;
        imageInfo.extent.height = m_imageInfo.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_imageInfo.mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = static_cast<VkFormat>(m_imageInfo.format);
        imageInfo.tiling = static_cast<VkImageTiling>(vk::ImageTiling::eOptimal);
        imageInfo.initialLayout = static_cast<VkImageLayout>(vk::ImageLayout::eUndefined);
        imageInfo.usage = static_cast<VkImageUsageFlags>(m_imageInfo.imageUsage);
        imageInfo.sharingMode = static_cast<VkSharingMode>(vk::SharingMode::eExclusive);
        imageInfo.samples = static_cast<VkSampleCountFlagBits>(m_imageInfo.samples);

        //m_imageVK = m_device.GetDevice().createImage(imageInfo);

        // create vma allocation
        VmaAllocationCreateInfo imageAllocationInfo = {};
        imageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        imageAllocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        assert(
            vmaCreateImage(m_device.GetAllocator(), &imageInfo, &imageAllocationInfo, &m_imageVMA, &m_imageAllocation,
                nullptr) == VK_SUCCESS);
        vmaSetAllocationName(m_device.GetAllocator(), m_imageAllocation, "<== IMAGE ==>");

        m_imageVK = m_imageVMA;
    }

    void VImage2::GenerateImageView()
    {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.image = m_imageVK;
        createInfo.format = m_imageInfo.format;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.subresourceRange.aspectMask = m_imageInfo.aspecFlags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = m_imageInfo.mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        m_imageView = m_device.GetDevice().createImageView(createInfo);
        assert(m_imageView);
        Utils::Logger::LogInfoVerboseOnly("2D Image view created");

    }
} // VulkanCore