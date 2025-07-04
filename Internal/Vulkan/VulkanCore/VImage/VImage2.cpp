//
// Created by wpsimon09 on 12/03/25.
//

#include "VImage2.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace VulkanCore {
VImage2::VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& info)
    : m_device(device)
    , m_imageInfo(info)
{
    if(info.arrayLayers == 6)
    {
        m_imageFlags.IsCubeMap = true;
    }
    else
        m_imageFlags.IsCubeMap = false;

    AllocateImage();
    GenerateImageView();
    m_imageFlags.IsStorage     = info.isStorage;
    m_imageFlags.IsDepthBuffer = IsDepth(m_imageInfo.format);
}

VImage2::VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& info, vk::Image swapChainImage)
    : m_device(device)
    , m_imageInfo(info)
    , m_imageVK(swapChainImage)
    , m_imageFlags{}
{
    m_imageFlags.IsSwapChainImage = true;

    GenerateImageView();
}

VImage2::VImage2(const VulkanCore::VDevice& device, VulkanStructs::VImageData<uint32_t>& imageData)
    : m_device(device)
    , m_imageInfo{}
    , m_imageFlags{}
{
    m_imageInfo.width       = imageData.widht;
    m_imageInfo.height      = imageData.height;
    m_imageInfo.imageSource = imageData.sourceType;
    m_imageInfo.format      = imageData.format;

    m_imageFlags.IsDepthBuffer = IsDepth(m_imageInfo.format);
    m_imageFlags.IsCubeMap     = m_imageInfo.arrayLayers == 6;

    AllocateImage();
    GenerateImageView();
    FillWithImageData(imageData, m_device.GetTransferOpsManager().GetCommandBuffer());
}

VImage2::VImage2(const VulkanCore::VDevice& device, VulkanStructs::VImageData<float>& imageData)
    : m_device(device)
    , m_imageInfo{}
    , m_imageFlags{}

{
    m_imageInfo.width       = imageData.widht;
    m_imageInfo.height      = imageData.height;
    m_imageInfo.imageSource = imageData.sourceType;
    m_imageInfo.format      = imageData.format;

    m_imageFlags.IsDepthBuffer = IsDepth(m_imageInfo.format);

    AllocateImage();
    GenerateImageView();
    FillWithImageData<float>(imageData, device.GetTransferOpsManager().GetCommandBuffer());
}
VImage2::VImage2(const VulkanCore::VDevice& device, std::vector<VulkanStructs::VImageData<float>>& imageDataArray)
    : m_device(device)
    , m_imageInfo{}
    , m_imageFlags{}
{
    //==============================================
    // this constructor will create the image array
    if(VulkanUtils::RelaxedAssert(!imageDataArray.empty(), "Image data are empty ! "))
    {
        auto& firstImage = imageDataArray[0];  // every image in the data array must be same except for the data wihin it, thus it is safe to take first image to populate the create info struct
        m_imageInfo.width             = firstImage.widht;
        m_imageInfo.height            = firstImage.height;
        m_imageInfo.arrayLayers       = imageDataArray.size();
        m_imageFlags.IsCubeMap        = false;
        m_imageFlags.IsSwapChainImage = false;
        m_imageInfo.channels          = firstImage.channels;
        m_imageInfo.format            = firstImage.format;
    }

    AllocateImage();
    GenerateImageView();
    FillWithImageData(imageDataArray, device.GetTransferOpsManager().GetCommandBuffer());
}

void VImage2::Resize(uint32_t newWidth, uint32_t newHeight)
{
    m_imageInfo.width  = newWidth;
    m_imageInfo.height = newHeight;
    Destroy();
    AllocateImage();
    GenerateImageView();
}

void VImage2::Destroy()
{
    if(!m_imageFlags.IsSwapChainImage)
    {
        vmaDestroyImage(m_device.GetAllocator(), m_imageVMA, m_imageAllocation);
    }
    m_device.GetDevice().destroyImageView(m_imageView);
    Utils::Logger::LogInfoVerboseOnly("Deleted image and its image view");
}

void VImage2::AllocateImage()
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType         = VK_IMAGE_TYPE_2D;
    imageInfo.flags             = m_imageFlags.IsCubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
    imageInfo.extent.width      = m_imageInfo.width;
    imageInfo.extent.height     = m_imageInfo.height;
    imageInfo.extent.depth      = 1;
    imageInfo.mipLevels         = m_imageInfo.mipLevels;
    imageInfo.arrayLayers       = m_imageInfo.arrayLayers;
    ;
    imageInfo.format        = static_cast<VkFormat>(m_imageInfo.format);
    imageInfo.tiling        = static_cast<VkImageTiling>(vk::ImageTiling::eOptimal);
    imageInfo.initialLayout = static_cast<VkImageLayout>(vk::ImageLayout::eUndefined);
    imageInfo.usage         = static_cast<VkImageUsageFlags>(m_imageInfo.imageUsage);
    imageInfo.sharingMode   = static_cast<VkSharingMode>(vk::SharingMode::eExclusive);
    imageInfo.samples       = static_cast<VkSampleCountFlagBits>(m_imageInfo.samples);
    m_imageSizeBytes = m_imageInfo.width * m_imageInfo.height * VulkanUtils::GetVulkanFormatSize(m_imageInfo.format)
                       * m_imageInfo.arrayLayers;

    // create vma allocation
    VmaAllocationCreateInfo imageAllocationInfo = {};
    imageAllocationInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    imageAllocationInfo.flags                   = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    VulkanUtils::Check(static_cast<vk::Result>(vmaCreateImage(m_device.GetAllocator(), &imageInfo, &imageAllocationInfo, &m_imageVMA, &m_imageAllocation, nullptr))
           ,static_cast<vk::Result>(VK_SUCCESS));

    vmaSetAllocationName(m_device.GetAllocator(), m_imageAllocation, m_imageInfo.imageAllocationName.c_str());

    m_imageVK = m_imageVMA;
}

void VImage2::GenerateImageView()
{
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image  = m_imageVK;
    createInfo.format = m_imageInfo.format;
    createInfo.viewType = EvaluateImageViewType();
    createInfo.subresourceRange.aspectMask     = m_imageInfo.aspecFlags;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = m_imageInfo.mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = m_imageInfo.arrayLayers;

    m_imageView = m_device.GetDevice().createImageView(createInfo);
    assert(m_imageView);
    Utils::Logger::LogInfoVerboseOnly("2D Image view created");
}

vk::ImageViewType VImage2::EvaluateImageViewType()
{
    if(m_imageFlags.IsCubeMap)
    {
        return vk::ImageViewType::eCube;
    }

    if(m_imageInfo.arrayLayers > 1 && !m_imageFlags.IsCubeMap)
    {
        return vk::ImageViewType::e2DArray;
    }
    return vk::ImageViewType::e2D;
}


bool VImage2::IsDepth(vk::Format& format)
{
    return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat || format == vk::Format::eD16UnormS8Uint
           || format == vk::Format::eD24UnormS8Uint || format == vk::Format::eD32SfloatS8Uint;
}


VImage2CreateInfo& VImage2::GetImageInfo()
{
    return m_imageInfo;
}

const VImage2CreateInfo& VImage2::GetImageInfoConstRef() const
{
    return m_imageInfo;
}

const VImage2Flags& VImage2::GetImageFlags() const
{
    return m_imageFlags;
}

vk::Image VImage2::GetImage() const
{
    return m_imageVK;
}

const vk::Image& VImage2::GetImageRef() const
{
    return m_imageVK;
}

vk::ImageView VImage2::GetImageView() const
{
    return m_imageView;
}

vk::DescriptorImageInfo VImage2::GetDescriptorImageInfo(vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = m_imageInfo.layout;
    imageInfo.imageView   = m_imageView;
    imageInfo.sampler     = sampler;
    return imageInfo;
}
vk::DescriptorImageInfo VImage2::GetDescriptorImageInfo()
{
    assert(m_imageFlags.IsStorage && "This function is only available for storage images !");
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = m_imageInfo.layout;
    imageInfo.imageView   = m_imageView;
    imageInfo.sampler     = nullptr;
    return imageInfo;
}
vk::ImageSubresourceRange VImage2::GetSubresrouceRange()
{
    vk::ImageSubresourceRange sub;
    sub.aspectMask = GetImageFlags().IsDepthBuffer ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil :
                                                     vk::ImageAspectFlagBits::eColor;
    sub.baseMipLevel   = 0;
    sub.levelCount     = m_imageInfo.mipLevels;
    sub.baseArrayLayer = 0;
    sub.layerCount     = m_imageInfo.arrayLayers;

    return sub;
}

vk::DeviceSize VImage2::GetImageSizeBytes()
{
    return m_imageSizeBytes;
}

VmaAllocation& VImage2::GetImageAllocation()
{
    return m_imageAllocation;
}

VmaAllocation VImage2::GetImageStagingBufferMemAllocation()
{
    return m_stagingBufferWithPixelData->GetStagingBufferAllocation();
}

vk::Buffer VImage2::GetImageStagingBuffer()
{
    return m_stagingBufferWithPixelData->GetStagingBuffer();
}

VulkanCore::VBuffer& VImage2::GetImageStagingvBuffer()
{
    return *m_stagingBufferWithPixelData;
}
}  // namespace VulkanCore
