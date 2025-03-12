//
// Created by wpsimon09 on 12/03/25.
//

#ifndef VIMAGE2_HPP
#define VIMAGE2_HPP
#include <glm/fwd.hpp>
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    class VBuffer;

    struct VImage2CreateInfo
{
    int width =  1;
    int height = 1;
    int channels = 4;
    int depth = 1;

    EImageSource imageSource = EImageSource::Generated;

    uint32_t mipLevels = 1;

    vk::Format format = vk::Format::eR8G8B8A8Srgb;
    vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor;
    vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eSampled;
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;
};

struct VImage2Flags
{
    bool IsDepthBuffer = false;
    bool IsSwapChainImage = false;
    bool IsValid = false;
    bool IsLoaded = false;
    bool IsSavable = false;
};

class VImage2: public VulkanCore::VObject {
    public:
        explicit VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& info);
        explicit VImage2(const VulkanCore::VDevice& device, const VImage2CreateInfo& createInfo, vk::Image swapChainImage);
        explicit VImage2(const VulkanCore::VDevice& device, VulkanStructs::ImageData<uint32_t>& imageData );

        void Resize(uint32_t newWidth, uint32_t newHeight, vk::CommandBuffer& cmdBuffer);
        void Destroy() override;

    private:
        VImage2CreateInfo m_imageInfo;
        VImage2Flags m_imageFlags;

        VkImage m_imageVMA;
        vk::Image m_imageVK;
        vk::ImageView m_imageView;
        VmaAllocation m_imageAllocation;

        vk::DeviceSize m_imageSizeBytes;
        std::unique_ptr<VulkanCore::VBuffer> m_stagingBufferWithPixelData;

        const VulkanCore::VDevice& m_device;

    private:
        void AllocateImage();
        void GenerateImageView();

};

} // VulkanCore

#endif //VIMAGE2_HPP
