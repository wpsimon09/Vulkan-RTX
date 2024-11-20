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
                        vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);

        // allocates new image from the image on the machine based on the provided falg it can be frame buffer
        explicit VImage(const VulkanCore::VDevice &device, uint32_t mipLevels = 1,
                        vk::Format format = vk::Format::eR8G8B8A8Unorm,
                        vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);


        void Resize(uint32_t newWidth, uint32_t newHeight) ;

        void Destroy() override;

        void TransitionImageLayout(vk::ImageLayout currentLayout, vk::ImageLayout targetLayout);

        bool IsValid() const {return m_isValid;}

        bool IsLoaded() const {return m_isLoaded;}

        void SetIsLoaded(bool status) {m_isLoaded = status;}

        void FillWithImageData(VulkanStrucuts::ImageData& imageData, bool transitionToShaderReadOnly = true);

        std::string GetPath() {return m_path;}

        vk::DescriptorImageInfo GetDescriptorImageInfo(vk::Sampler &sampler);

        ~VImage() = default;

    private:
        void GenerateImage();
        void CopyFromBufferToImage();
        void GenerateImageView();
        void AllocateImage(size_t imageSize);

    private:
        const VulkanCore::VDevice &m_device;

        VkImage m_imageVMA;
        vk::Image m_imageVK;
        vk::ImageView m_imageView;
        vk::Sampler m_sampler;
        vk::Format m_format;
        vk::ImageAspectFlags m_aspectFlags;
        vk::DeviceSize m_imageSize;
        vk::ImageLayout m_imageLayout;
        vk::Sampler m_imageSampler;
        VmaAllocation m_imageAllocation;

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
}


#endif //VIMAGE_HPP
