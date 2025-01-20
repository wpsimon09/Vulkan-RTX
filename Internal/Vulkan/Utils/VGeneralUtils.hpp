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

#include "fastgltf/base64.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace ApplicationCore
{
    struct Vertex;
}

namespace VulkanCore
{
    class MeshDatatManager;
}

struct TextureBufferInfo;

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

    VulkanStructs::ImageData LoadImage(const std::string &path);
    VulkanStructs::ImageData LoadImage(const TextureBufferInfo& data, const std::string& textureID);

    std::string BufferUsageFlagToString(vk::BufferUsageFlags usage);

    glm::mat4 FastGLTFToGLMMat4(fastgltf::math::fmat4x4& matrix);

    std::pair<vk::Result, uint32_t> SwapChainNextImageKHRWrapper(const VulkanCore::VDevice& device,const VulkanCore::VSwapChain& swapChain, uint64_t timeOut, const VulkanCore::VSyncPrimitive<vk::Semaphore>& semaphore, VulkanCore::VSyncPrimitive<vk::Fence>* fence);

    vk::Result PresentQueueWrapper(vk::Queue queue, const vk::PresentInfoKHR &presentInfo);

    std::string random_string( size_t length );

    VulkanStructs::Bounds CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices);

    VulkanStructs::StagingBufferInfo CreateStagingBuffer(const VulkanCore::VDevice& m_device, vk::DeviceSize size);

    bool IsInViewFrustum(VulkanStructs::Bounds* bounds, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
}

#endif //VGENERALUTILS_HPP
