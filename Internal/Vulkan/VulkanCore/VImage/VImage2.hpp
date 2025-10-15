//
// Created by wpsimon09 on 12/03/25.
//

#ifndef VIMAGE2_HPP
#define VIMAGE2_HPP
#include <cstdint>
#include <glm/fwd.hpp>
#include <string>
#include <vulkan/vulkan.hpp>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
class VBuffer;

struct VImage2CreateInfo
{
    int width    = 1;
    int height   = 1;
    int channels = 4;
    int depth    = 1;

    std::filesystem::path imagePath;
    EImageSource          imageSource = EImageSource::Generated;

    uint32_t mipLevels   = 1;
    uint32_t arrayLayers = 1;

    vk::Format              format     = vk::Format::eR8G8B8A8Srgb;
    vk::ImageAspectFlags    aspecFlags = vk::ImageAspectFlagBits::eColor;
    vk::ImageUsageFlags     imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    vk::SampleCountFlagBits samples    = vk::SampleCountFlagBits::e1;
    vk::ImageLayout         layout     = vk::ImageLayout::eUndefined;

    std::string imageAllocationName = "Image allocation name not set !";
    std::string imageDebugName      = "";

    bool isStorage = false;

    void SetImageLayout(vk::ImageLayout newLayout) { layout = newLayout; }
    void SetWidth(int newWidth) { width = newWidth; }
    void SetHeight(int newHeight) { height = newHeight; }
    void SetChannels(int newChannels) { channels = newChannels; }
    void SetDepth(int newDepth) { depth = newDepth; }
    void SetImagePath(const std::filesystem::path& newPath) { imagePath = newPath; }
    void SetImageSource(EImageSource newSource) { imageSource = newSource; }
    void SetMipLevels(uint32_t newMipLevels) { mipLevels = newMipLevels; }
    void SetFormat(vk::Format newFormat) { format = newFormat; }
    void SetAspectFlags(vk::ImageAspectFlags newAspectFlags) { aspecFlags = newAspectFlags; }
    void SetImageUsage(vk::ImageUsageFlags newImageUsage) { imageUsage = newImageUsage; }
    void SetSamples(vk::SampleCountFlagBits newSamples) { samples = newSamples; }
    void SetLayout(vk::ImageLayout newLayout) { layout = newLayout; }
};

struct VImage2Flags
{
    bool IsDepthBuffer    = false;
    bool IsCubeMap        = false;
    bool IsSwapChainImage = false;
    bool IsValid          = false;
    bool IsLoaded         = false;
    bool IsSavable        = false;
    bool IsStorage        = false;
};

class VImage2 : public VulkanCore::VObject
{
  public:
    explicit VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& info);
    explicit VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& createInfo, vk::Image swapChainImage);
    explicit VImage2(const VulkanCore::VDevice& device, VulkanStructs::VImageData<uint32_t>& imageData);
    explicit VImage2(const VulkanCore::VDevice& device, VulkanStructs::VImageData<float>& imageData);
    explicit VImage2(const VulkanCore::VDevice& device, std::vector<VulkanStructs::VImageData<float>>& imageDataArray);

    void Resize(uint32_t newWidth, uint32_t newHeight);
    template <typename T>
    void FillWithImageData(const VulkanStructs::VImageData<T>& imageData,
                           VulkanCore::VCommandBuffer&         cmdBuffer,
                           bool                                transitionToShaderReadOnly = true,
                           bool                                destroyCurrentImage        = false);

    template <typename T>
    void FillWithImageData(const std::vector<VulkanStructs::VImageData<T>>& imageData,
                           VulkanCore::VCommandBuffer&                      cmdBuffer,
                           bool                                             transitionToShaderReadOnly = true,
                           bool                                             destroyCurrentImage        = false);

    void Destroy() override;

    VImage2CreateInfo&        GetImageInfo();
    const VImage2CreateInfo&  GetImageInfoConstRef() const;
    const VImage2Flags&       GetImageFlags() const;
    vk::Image                 GetImage() const;
    const vk::Image&          GetImageRef() const;
    vk::ImageView             GetImageView() const;
    vk::DescriptorImageInfo   GetDescriptorImageInfo(vk::Sampler& sampler);
    vk::DescriptorImageInfo   GetDescriptorImageInfo();
    vk::ImageSubresourceRange GetSubresrouceRange();
    uint32_t                  GetPixelCount();
    void                      GiveDebugName(const std::string& name);

    vk::DeviceSize GetImageSizeBytes();
    VmaAllocation& GetImageAllocation();

    VmaAllocation        GetImageStagingBufferMemAllocation();
    vk::Buffer           GetImageStagingBuffer();
    VulkanCore::VBuffer& GetImageStagingvBuffer();


  private:
    VImage2CreateInfo m_imageInfo;
    VImage2Flags      m_imageFlags;

    VkImage       m_imageVMA;
    vk::Image     m_imageVK;
    vk::ImageView m_imageView;
    VmaAllocation m_imageAllocation;

    vk::DeviceSize m_imageSizeBytes;

    const VulkanCore::VDevice& m_device;

    std::unique_ptr<VulkanCore::VBuffer> m_stagingBufferWithPixelData;

  private:
    void              AllocateImage();
    void              GenerateImageView();
    vk::ImageViewType EvaluateImageViewType();
    bool              IsDepth(vk::Format& format);
    bool              IsCube;
};

//========================================================================================================================================
// for use with singel image
//========================================================================================================================================


template <typename T>
void VImage2::FillWithImageData(const VulkanStructs::VImageData<T>& imageData,
                                VulkanCore::VCommandBuffer&         cmdBuffer,
                                bool                                transitionToShaderReadOnly,
                                bool                                destroyCurrentImage)
{

    if(!imageData.pixels)
    {
        Utils::Logger::LogError("Image pixel data are corrupted ! ");
        return;
    }

    if(destroyCurrentImage)
    {
        Resize(imageData.widht, imageData.height);
    }
    //m_imageInfo.P = imageData.fileName;
    // copy pixel data to the staging buffer

    assert(cmdBuffer.GetIsRecording()
           && "Command buffer is not recording any commands, before using it make sure it is in recording state  !");

    Utils::Logger::LogInfoVerboseOnly("Copying image data to staging buffer");

    m_stagingBufferWithPixelData = std::make_unique<VulkanCore::VBuffer>(m_device, "<== IMAGE STAGING BUFFER ==>");
    m_stagingBufferWithPixelData->CreateHostVisibleBuffer(imageData.GetSize());

    memcpy(m_stagingBufferWithPixelData->MapStagingBuffer(), imageData.pixels, imageData.GetSize());
    m_stagingBufferWithPixelData->UnMapStagingBuffer();

    Utils::Logger::LogInfoVerboseOnly("Image data copied");

    // transition image to the transfer dst optimal layout so that data can be copied to it
    VulkanUtils::PlaceImageMemoryBarrier2(*this, cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                          VulkanUtils::VImage_Undefined_ToTransferDst);
    ;
    //TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    vk::BufferImageCopy region = {};
    region.bufferOffset        = 0;
    region.bufferRowLength     = 0;
    region.bufferImageHeight   = 0;

    region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;

    region.imageExtent.width  = m_imageInfo.width;
    region.imageExtent.height = m_imageInfo.height;
    region.imageExtent.depth  = m_imageInfo.depth;

    cmdBuffer.GetCommandBuffer().copyBufferToImage(m_stagingBufferWithPixelData->GetStagingBuffer(), m_imageVK,
                                                   vk::ImageLayout::eTransferDstOptimal, 1, &region);

    Utils::Logger::LogInfoVerboseOnly("Flag transitionToShaderReadOnly is true, executing transition...");
    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
                                                vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader,
                                                vk::AccessFlagBits2::eShaderRead};

    PlaceImageMemoryBarrier2(*this, cmdBuffer, vk::ImageLayout::eTransferDstOptimal,
                             vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);

    // this should be safe, since data are in staging
    imageData.Clear();
}

//========================================================================================================================================
// for use with array of image data, this will create texture array
//========================================================================================================================================

template <typename T>
void VImage2::FillWithImageData(const std::vector<VulkanStructs::VImageData<T>>& imageData,
                                VulkanCore::VCommandBuffer&                      cmdBuffer,
                                bool                                             transitionToShaderReadOnly,
                                bool                                             destroyCurrentImage)
{
    //===================================
    // accumulate size for staging buffer
    vk::DeviceSize                   offset{0};
    std::vector<vk::BufferImageCopy> copyRegions(imageData.size());

    m_stagingBufferWithPixelData = std::make_unique<VulkanCore::VBuffer>(m_device, "<== IMAGE ARRAY STAGING BUFFER ==>");
    m_stagingBufferWithPixelData->CreateHostVisibleBuffer(m_imageSizeBytes);

    auto mappedStaginBuffer = m_stagingBufferWithPixelData->MapStagingBuffer();

    for(int layer = 0; layer < imageData.size(); layer++)
    {
        if(!imageData[layer].pixels)
        {
            Utils::Logger::LogError("Image pixel data are corrupted ! ");
            return;
        }


        // copy pixel data to the staging buffer
        assert(cmdBuffer.GetIsRecording()
               && "Command buffer is not recording any commands, before using it make sure it is in recording state  !");

        Utils::Logger::LogInfoVerboseOnly("Copying image data to staging buffer");

        memcpy(mappedStaginBuffer + offset, imageData[layer].pixels, imageData[layer].GetSize());

        Utils::Logger::LogInfoVerboseOnly("Image data copied");

        vk::BufferImageCopy region = {};
        region.bufferOffset        = offset;

        region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount     = 1;

        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;

        region.imageExtent.width  = m_imageInfo.width;
        region.imageExtent.height = m_imageInfo.height;
        region.imageExtent.depth  = m_imageInfo.depth;

        copyRegions[layer] = region;

        offset += imageData[layer].GetSize();


        // this should be safe, since data are in staging
        imageData[layer].Clear();
    }
    m_stagingBufferWithPixelData->UnMapStagingBuffer();

    // transition image to the transfer dst optimal layout so that data can be copied to it
    VulkanUtils::PlaceImageMemoryBarrier2(*this, cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                          VulkanUtils::VImage_Undefined_ToTransferDst);
    ;

    cmdBuffer.GetCommandBuffer().copyBufferToImage(m_stagingBufferWithPixelData->GetStagingBuffer(), m_imageVK,
                                                   vk::ImageLayout::eTransferDstOptimal, copyRegions.size(), copyRegions.data());


    Utils::Logger::LogInfoVerboseOnly("Flag transitionToShaderReadOnly is true, executing transition...");

    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
                                                vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader,
                                                vk::AccessFlagBits2::eShaderRead};

    VulkanUtils::PlaceImageMemoryBarrier2(*this, cmdBuffer, vk::ImageLayout::eTransferDstOptimal,
                                          vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);

    //imageData.clear();
}

}  // namespace VulkanCore

#endif  //VIMAGE2_HPP
