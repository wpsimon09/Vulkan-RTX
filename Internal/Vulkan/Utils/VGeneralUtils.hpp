//
// Created by wpsimon09 on 27/09/24.
//

#ifndef VGENERALUTILS_HPP
#define VGENERALUTILS_HPP
#include <cstdint>
#include <memory>
#include <stb_image/stb_image.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanCore
{
    class VSwapChain;
    class VDevice;
    class VCommandPool;
}

namespace ApplicationCore
{
    class VertexArray;
}

namespace VulkanUtils
{
    uint32_t FindQueueFamily(const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties, vk::QueueFlagBits queueType);
    vk::ImageView GenerateImageView(const vk::Device& logicalDevice, const vk::Image& image, uint32_t mipLevels = 1, vk::Format format = vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);

    void GetVertexBindingAndAttributeDescription(vk::VertexInputBindingDescription &bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> &attributeDescription
    );

    void CopyBuffers(const VulkanCore::VDevice& device, const vk::Buffer &srcBuffer, const vk::Buffer &dstBuffer, vk::DeviceSize size);

    VulkanStrucuts::ImageData LoadImage(const std::string &path);

    std::pair<vk::Result, uint32_t> SwapChainNextImageKHRWrapper(const VulkanCore::VDevice& device,const VulkanCore::VSwapChain& swapChain, uint64_t timeOut, const VulkanCore::VSyncPrimitive<vk::Semaphore>& semaphore, VulkanCore::VSyncPrimitive<vk::Fence>* fence);

    vk::Result PresentQueueWrapper(vk::Queue queue, const vk::PresentInfoKHR &presentInfo);
}

#endif //VGENERALUTILS_HPP
