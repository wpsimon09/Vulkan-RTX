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
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanUtils {
class VEffect;
}
namespace VulkanCore {
class VDescriptorAllocator;
}
namespace ApplicationCore {
struct Vertex;
}

namespace VulkanCore {
class MeshDatatManager;
}

struct TextureBufferInfo;

namespace VulkanCore {
class VSwapChain;
class VDevice;
class VCommandPool;
}  // namespace VulkanCore

namespace ApplicationCore {
class VertexArray;
}

namespace VulkanUtils {
uint32_t FindQueueFamily(const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties, vk::QueueFlagBits queueType);

vk::ImageView GenerateImageView(const vk::Device&    logicalDevice,
                                const vk::Image&     image,
                                uint32_t             mipLevels  = 1,
                                vk::Format           format     = vk::Format::eR8G8B8A8Srgb,
                                vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);

void GetVertexBindingAndAttributeDescription(vk::VertexInputBindingDescription&                bindingDescription,
                                             std::vector<vk::VertexInputAttributeDescription>& attributeDescription,
                                             EVertexInput                                      vertexInput = Full);

void CopyBuffers(const VulkanCore::VDevice&                   device,
                 const VulkanCore::VSyncPrimitive<vk::Fence>& fence,
                 const vk::Buffer&                            srcBuffer,
                 const vk::Buffer&                            dstBuffer,
                 vk::DeviceSize                               size,
                 vk::DeviceSize                               srcOffset = 0,
                 vk::DeviceSize                               dstOffset = 0);

// records copy command to the command buffer provided, usually this is called during acceleration structure build
void CopyBuffers(const vk::CommandBuffer& commandBuffer,
                 const vk::Buffer&        srcBuffer,
                 const vk::Buffer&        dstBuffer,
                 vk::DeviceSize           size,
                 vk::DeviceSize           srcOffset = 0,
                 vk::DeviceSize           dstOffset = 0);

void CopyBuffersWithBariers(const VulkanCore::VDevice& device,
                            const vk::Buffer&          srcBuffer,
                            const vk::Buffer&          dstBuffer,
                            vk::DeviceSize             size,
                            vk::DeviceSize             srcOffset = 0,
                            vk::DeviceSize             dstOffset = 0);

std::string BufferUsageFlagToString(vk::BufferUsageFlags usage);

std::pair<vk::Result, uint32_t> SwapChainNextImageKHRWrapper(const VulkanCore::VDevice&                       device,
                                                             const VulkanCore::VSwapChain&                    swapChain,
                                                             uint64_t                                         timeOut,
                                                             const VulkanCore::VSyncPrimitive<vk::Semaphore>& semaphore,
                                                             VulkanCore::VSyncPrimitive<vk::Fence>*           fence);

vk::Result PresentQueueWrapper(vk::Queue queue, const vk::PresentInfoKHR& presentInfo);

std::string random_string(size_t length);

int random_int(int min = 0, int max = 1);

vk::DeviceSize GetVulkanFormatSize(vk::Format format);

VulkanStructs::VStagingBufferInfo CreateStagingBuffer(const VulkanCore::VDevice& m_device, vk::DeviceSize size);

bool IsInViewFrustum(VulkanStructs::VBounds* bounds, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

int vkSampleToInt(vk::SampleCountFlagBits sample);

vk::SampleCountFlagBits IntToVkSample(int sampleCount);

std::vector<char> ReadSPIRVShader(std::filesystem::path shaderPath);

vk::ShaderModule CreateShaderModule(const VulkanCore::VDevice& device, const std::vector<char>& data);

void Check(vk::Result result, vk::Result expectedResult = vk::Result::eSuccess);

vk::DescriptorPool CreatePool(const VulkanCore::VDevice& devic, const VulkanCore::VDescriptorAllocator::PoolSizes& poolSizes, int count, vk::DescriptorPoolCreateFlags flags);

void WriteMaterialToDescriptorSet(ApplicationCore::BaseMaterial* mat, VEffect& effect);

std::string DescriptorTypeToString(vk::DescriptorType descriptorType);

}  // namespace VulkanUtils

#endif  //VGENERALUTILS_HPP
