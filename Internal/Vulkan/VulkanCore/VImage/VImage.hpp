//
// Created by wpsimon09 on 12/10/24.
//

#ifndef VIMAGE_HPP
#define VIMAGE_HPP
#include <VMA/vk_mem_alloc.h>
#include <stb_image/stb_image.h>
#include <vulkan/vulkan.hpp>
#include "memory"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"


namespace VulkanCore
{
    class VBuffer;
    class VCommandBuffer;
    class VDevice;
}

namespace VulkanCore
{
    class VImage : public VulkanCore::VObject
    {
    public:
        // creates image and iamge views from existing image, mostly used for retrieving SwapChain images
        explicit VImage(const VulkanCore::VDevice &device, vk::Image image, int widht, int height,
                        uint32_t mipLevels = 1, vk::Format format = vk::Format::eR8G8B8A8Srgb,
                        vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor,
                        std::optional<vk::ImageUsageFlags> imageUsage = std::nullopt,
                        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

        // allocates new image from the image on the machine based on the provided falg it can be frame buffer
        explicit VImage(const VulkanCore::VDevice &device, uint32_t mipLevels = 1,
                        vk::Format format = vk::Format::eR8G8B8A8Unorm,
                        vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor,
                        std::optional<vk::ImageUsageFlags> imageUsage = std::nullopt,
                        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

        void Resize(uint32_t newWidth, uint32_t newHeight) ;

        void Destroy() override;

        void TransitionImageLayout(vk::ImageLayout currentLayout, vk::ImageLayout targetLayout);

        void TransitionImageLayout(vk::ImageLayout currentLayout, vk::ImageLayout targetLayout, VulkanCore::VCommandBuffer& commandBuffer);

        void TransitionImageLayout(vk::ImageLayout currentLayout, vk::ImageLayout targetLayout, std::vector<vk::Semaphore>& waitSemaphores,std::vector<vk::PipelineStageFlags>& waitStages, std::vector<vk::Semaphore>& signalSemaphores);

        bool IsValid() const {return m_isValid;}

        bool IsLoaded() const {return m_isLoaded;}

        vk::DeviceSize GetSize() const {return m_width * m_height * VulkanUtils::GetVulkanFormatSize(m_format);}

        void SetIsLoaded(bool status) {m_isLoaded = status;}

        template<typename T>
        void FillWithImageData(const VulkanStructs::ImageData<T>& imageData, bool transitionToShaderReadOnly = true, bool destroyCurrentImage = false);

        std::string GetPath() {return m_path;}
        void SetPath(const std::string& path) {m_path = path;}  
        const vk::ImageLayout& GetCurrentLayout() const {return m_imageLayout;};

        vk::DescriptorImageInfo GetDescriptorImageInfo(vk::Sampler &sampler);

        ~VImage() = default;

    private:
        void CopyFromBufferToImage();
        void GenerateImageView();
        void AllocateImage(size_t imageSize);


    private:
        const VulkanCore::VDevice &m_device;

        VkImage m_imageVMA;
        vk::Image m_imageVK;
        vk::ImageView m_imageView;
        vk::Format m_format;
        vk::ImageAspectFlags m_aspectFlags;
        vk::DeviceSize m_imageSize;
        vk::ImageLayout m_imageLayout;
        VmaAllocation m_imageAllocation;
        vk::SampleCountFlagBits m_samples;


        uint32_t m_mipLevels;
        int m_width, m_height;

        // boolean flags
        bool m_isDepthBuffer;
        bool m_isSwapChainImage = false;
        bool m_isValid = false;
        bool m_isLoaded = false;

        // path for the future reference
        std::string m_path;

        std::unique_ptr<VulkanCore::VBuffer> m_stagingBufferWithPixelData;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCommandBuffer;

        std::optional<vk::ImageUsageFlags> m_imageUsage;

    public:
        const bool &GetIsSwapChainImage() const { return m_isSwapChainImage; };

        const bool GetIsDepthBuffer() const { return m_isDepthBuffer; };

        [[nodiscard]] const vk::Image &GetImage() const {
            return m_imageVK;
        }

        [[nodiscard]] const vk::ImageView &GetImageView() const {
            return m_imageView;
        }

        [[nodiscard]] uint32_t GetMipLevelels() const {
            return m_mipLevels;
        }

        [[nodiscard]] vk::Format GetFormat() const {
            return m_format;
        }

        [[nodiscard]] const vk::ImageAspectFlags &GetAspectFlag() const {
            return m_aspectFlags;
        }

        [[nodiscard]] int GetWidth() const {
            return m_width;
        }

        [[nodiscard]] int GetHeight() const {
            return m_height;
        }
    };

    template <typename T>
    void VImage::FillWithImageData(const VulkanStructs::ImageData<T>& imageData, bool transitionToShaderReadOnly,
        bool destroyCurrentImage)
    {
        if(destroyCurrentImage)
        {
            Resize(imageData.widht, imageData.height);
        }
        m_path = imageData.fileName;

        m_transferCommandBuffer->BeginRecording();
        // copy pixel data to the staging buffer
        Utils::Logger::LogInfoVerboseOnly("Copying image data to staging buffer");

        m_stagingBufferWithPixelData = std::make_unique<VulkanCore::VBuffer>(m_device, "<== IMAGE STAGING BUFFER ==>" + m_path);
        m_stagingBufferWithPixelData->CreateStagingBuffer(imageData.GetSize());

        memcpy(m_stagingBufferWithPixelData->MapStagingBuffer(), imageData.pixels, imageData.GetSize());
        m_stagingBufferWithPixelData->UnMapStagingBuffer();

        Utils::Logger::LogInfoVerboseOnly("Image data copied");

        // transition image to the transfer dst optimal layout so that data can be copied to it
        TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        CopyFromBufferToImage();

        if(!transitionToShaderReadOnly) {
            Utils::Logger::LogInfoVerboseOnly("Flag transitionToShaderReadOnly is false, this image will remain in Dst copy layout !");
            m_transferCommandBuffer->EndAndFlush(m_device.GetTransferQueue());
            return;
        }
        Utils::Logger::LogInfoVerboseOnly("Flag transitionToShaderReadOnly is true, executing transition...");
        TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        // execute the recorded commands
        m_transferCommandBuffer->EndAndFlush(m_device.GetTransferQueue());

        m_device.GetTransferQueue().waitIdle();
        m_stagingBufferWithPixelData->DestroyStagingBuffer();

    }
}


#endif //VIMAGE_HPP
