//
// Created by wpsimon09 on 27/09/24.
//

#include "VGeneralUtils.hpp"
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

#include <thread>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Global/GlobalState.hpp"

#include <fstream>

uint32_t VulkanUtils::FindQueueFamily(const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties, vk::QueueFlagBits queueType)
{
    /**
     * TODO: this is the function that will find proper queue families however i was stupid and thought that transfer family is different from graphics family
     * and for this reason everything that was happeing "on transfer queue" was acctualy happening on graphics queue
     *
     * I have to rewrite good portion of vulkan core functionality that handles image transition, copying and more to separate asnychronus queues because
     * transfer queue has no idea about shader stages and stuff like this. I will most likely create class similuar to VTransferOpsManager that will hanlde purely initial image layout transitions and
     * during rendering I will have like BeginFrame function that will return command buffer that can be recorded. This command buffer will also be used for image layout transitionss
     ***/
    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
        const auto& queueFamily = queueFamilyProperties[i];

        switch (queueType) {
        case vk::QueueFlagBits::eGraphics:
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                return i;
            }
            break;
        /**
        case vk::QueueFlagBits::eTransfer:
            // Prefer a dedicated transfer queue (not also graphics or compute)
                if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) &&
                    !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) &&
                    !(queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
                    return i;
                    }
            break;
        */
        case vk::QueueFlagBits::eCompute:
            // Prefer a dedicated compute queue (not also graphics)
                if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
                    !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
                    return i;
                    }
            break;

        default:
            break;
        }
    }

    // Fallback: Try to find any queue that supports the requested type, even if not dedicated
    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
        const auto& queueFamily = queueFamilyProperties[i];
        if (queueFamily.queueFlags & queueType) {
            return i;
        }
    }

    //select just the queue fmily index that supports graphics operations
    std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty =
        std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
                     [queueType](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & queueType; });

    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    auto queueFamilyIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    Utils::Logger::LogInfoVerboseOnly("Found graphics queue family at index: " + std::to_string(queueFamilyIndex));
    return queueFamilyIndex;
    }






vk::ImageView VulkanUtils::GenerateImageView(const vk::Device&    logicalDevice,
                                             const vk::Image&     image,
                                             uint32_t             mipLevels,
                                             vk::Format           format,
                                             vk::ImageAspectFlags aspecFlags)
{
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image                           = image;
    createInfo.format                          = format;
    createInfo.viewType                        = vk::ImageViewType::e2D;
    createInfo.subresourceRange.aspectMask     = aspecFlags;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    vk::ImageView imageView = logicalDevice.createImageView(createInfo);
    assert(imageView != VK_NULL_HANDLE);
    Utils::Logger::LogInfoVerboseOnly("2D Image view created [this message was generated by utility function]");
    return imageView;
}

void VulkanUtils::GetVertexBindingAndAttributeDescription(vk::VertexInputBindingDescription& bindingDescription,
                                                          std::vector<vk::VertexInputAttributeDescription>& attributeDescription,
                                                          EVertexInput vertexInput)
{
    bindingDescription.binding   = 0;
    bindingDescription.stride    = sizeof(ApplicationCore::Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;


    switch(vertexInput)
    {
        case Full: {
            attributeDescription.resize(3);  // enum specifies the number

            // FOR POSITION
            attributeDescription[0].binding  = 0;
            attributeDescription[0].location = 0;
            attributeDescription[0].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescription[0].offset   = offsetof(ApplicationCore::Vertex, position);

            // NORMALS
            attributeDescription[1].binding  = 0;
            attributeDescription[1].location = 1;
            attributeDescription[1].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescription[1].offset   = offsetof(ApplicationCore::Vertex, normal);

            // uv
            attributeDescription[2].binding  = 0;
            attributeDescription[2].location = 2;
            attributeDescription[2].format   = vk::Format::eR32G32Sfloat;
            attributeDescription[2].offset   = offsetof(ApplicationCore::Vertex, uv);

            break;
        }
        case Position_UV: {
            attributeDescription.resize(2);  // enum specifies the number

            // FOR POSITION
            attributeDescription[0].binding  = 0;
            attributeDescription[0].location = 0;
            attributeDescription[0].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescription[0].offset   = offsetof(ApplicationCore::Vertex, position);

            // uv
            attributeDescription[1].binding  = 0;
            attributeDescription[1].location = 1;
            attributeDescription[1].format   = vk::Format::eR32G32Sfloat;
            attributeDescription[1].offset   = offsetof(ApplicationCore::Vertex, uv);

            break;
        };
        case Position_Normal: {
            attributeDescription.resize(2);  // enum specifies the number

            // FOR POSITION
            attributeDescription[0].binding  = 0;
            attributeDescription[0].location = 0;
            attributeDescription[0].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescription[0].offset   = offsetof(ApplicationCore::Vertex, position);

            // NORMALS
            attributeDescription[1].binding  = 0;
            attributeDescription[1].location = 1;
            attributeDescription[1].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescription[1].offset   = offsetof(ApplicationCore::Vertex, normal);

            break;
        }
        case PositionOnly: {
            attributeDescription.resize(1);  // enum specifies the number

            // FOR POSITION
            attributeDescription[0].binding  = 0;
            attributeDescription[0].location = 0;
            attributeDescription[0].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescription[0].offset   = offsetof(ApplicationCore::Vertex, position);

            break;
        }
        case Empty: {
        }
    }
}

void VulkanUtils::CopyBuffers(const VulkanCore::VDevice&                   device,
                              const VulkanCore::VSyncPrimitive<vk::Fence>& fence,
                              const vk::Buffer&                            srcBuffer,
                              const vk::Buffer&                            dstBuffer,
                              vk::DeviceSize                               size,
                              vk::DeviceSize                               srcOffset,
                              vk::DeviceSize                               dstOffset)
{


    auto& comandPool = device.GetTransferCommandPool();
    auto  cmdBuffer  = VulkanCore::VCommandBuffer(device, comandPool);
    Utils::Logger::LogInfoVerboseOnly("Copying buffers...");

    cmdBuffer.BeginRecording();

    vk::BufferCopy bufferCopy{};
    bufferCopy.srcOffset = srcOffset;
    bufferCopy.dstOffset = dstOffset;
    bufferCopy.size      = size;

    cmdBuffer.GetCommandBuffer().copyBuffer(srcBuffer, dstBuffer, bufferCopy);

    cmdBuffer.EndRecording();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmdBuffer.GetCommandBuffer();

    assert(device.GetTransferQueue().submit(1, &submitInfo, fence.GetSyncPrimitive()) == vk::Result::eSuccess
           && "Failed to submit command buffer ");

    fence.WaitForFence();
    comandPool.SetInUse(false);

    Utils::Logger::LogSuccess("Buffer copy completed !");
}
void VulkanUtils::CopyBuffers(const vk::CommandBuffer& commandBuffer,
                              const vk::Buffer&        srcBuffer,
                              const vk::Buffer&        dstBuffer,
                              vk::DeviceSize           size,
                              vk::DeviceSize           srcOffset,
                              vk::DeviceSize           dstOffset)
{
    Utils::Logger::LogInfoVerboseOnly("Copying buffers...");

    if (size <= 0 ) return;

    vk::BufferCopy bufferCopy{};
    bufferCopy.srcOffset = srcOffset;
    bufferCopy.dstOffset = dstOffset;
    bufferCopy.size      = size;

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, bufferCopy);

    Utils::Logger::LogInfoVerboseOnly("Buffer copy completed !");
}

void VulkanUtils::CopyBuffersWithBariers(const VulkanCore::VDevice& device,
                                         const vk::Buffer&          srcBuffer,
                                         const vk::Buffer&          dstBuffer,
                                         vk::DeviceSize             size,
                                         vk::DeviceSize             srcOffset,
                                         vk::DeviceSize             dstOffset)
{
    auto& commadnPool = device.GetTransferCommandPool();
    auto  cmdBuffer   = VulkanCore::VCommandBuffer(device, commadnPool);
    auto  fence       = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(device);

    Utils::Logger::LogInfoVerboseOnly("Copying buffers...");

    cmdBuffer.BeginRecording();

    vk::BufferCopy bufferCopy{};
    bufferCopy.srcOffset = srcOffset;
    bufferCopy.dstOffset = dstOffset;
    bufferCopy.size      = size;

    cmdBuffer.GetCommandBuffer().copyBuffer(srcBuffer, dstBuffer, bufferCopy);

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmdBuffer.GetCommandBuffer();

    assert(device.GetTransferQueue().submit(1, &submitInfo, fence->GetSyncPrimitive()) == vk::Result::eSuccess);
    Utils::Logger::LogSuccess("Buffer copy completed !");
    fence->WaitForFence();
    commadnPool.SetInUse(false);
    fence->Destroy();
}


std::string VulkanUtils::BufferUsageFlagToString(vk::BufferUsageFlags usage)
{
    std::string result;

    if(usage & vk::BufferUsageFlagBits::eTransferSrc)
    {
        result += "Transfer Source | ";
    }
    if(usage & vk::BufferUsageFlagBits::eTransferDst)
    {
        result += "Transfer Destination | ";
    }
    if(usage & vk::BufferUsageFlagBits::eUniformTexelBuffer)
    {
        result += "Uniform Texel Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eStorageTexelBuffer)
    {
        result += "Storage Texel Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eUniformBuffer)
    {
        result += "Uniform Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eStorageBuffer)
    {
        result += "Storage Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eIndexBuffer)
    {
        result += "Index Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eVertexBuffer)
    {
        result += "Vertex Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eIndirectBuffer)
    {
        result += "Indirect Buffer | ";
    }
    if(usage & vk::BufferUsageFlagBits::eShaderDeviceAddress)
    {
        result += "Shader Device Address | ";
    }
    if(usage & vk::BufferUsageFlagBits::eShaderDeviceAddressEXT)
    {
        result += "Shader Device Address EXT | ";
    }
    if(usage & vk::BufferUsageFlagBits::eShaderDeviceAddressKHR)
    {
        result += "Shader Device Address KHR | ";
    }
    if(usage & vk::BufferUsageFlagBits::eVideoDecodeSrcKHR)
    {
        result += "Video Decode Source KHR | ";
    }
    if(usage & vk::BufferUsageFlagBits::eVideoDecodeDstKHR)
    {
        result += "Video Decode Destination KHR | ";
    }
    if(usage & vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT)
    {
        result += "Transform Feedback Buffer EXT | ";
    }
    if(usage & vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT)
    {
        result += "Transform Feedback Counter Buffer EXT | ";
    }
    if(usage & vk::BufferUsageFlagBits::eConditionalRenderingEXT)
    {
        result += "Conditional Rendering EXT | ";
    }

    if(!result.empty())
    {
        result.erase(result.size() - 3);
    }
    else
    {
        result = "Unknown Buffer Usage";
    }

    return result;
}

std::pair<vk::Result, uint32_t> VulkanUtils::SwapChainNextImageKHRWrapper(const VulkanCore::VDevice&    device,
                                                                          const VulkanCore::VSwapChain& swapChain,
                                                                          uint64_t                      timeOut,
                                                                          const VulkanCore::VSyncPrimitive<vk::Semaphore>& semaphore,
                                                                          VulkanCore::VSyncPrimitive<vk::Fence>* fence)
{
    uint32_t image_index;
    auto result = static_cast<vk::Result>(vkAcquireNextImageKHR(device.GetDevice(), swapChain.GetSwapChain(), timeOut,
                                                                semaphore.GetSyncPrimitive(), nullptr, &image_index));
    return std::make_pair(result, image_index);
}

vk::Result VulkanUtils::PresentQueueWrapper(vk::Queue queue, const vk::PresentInfoKHR& presentInfo)
{
    auto result = static_cast<vk::Result>(vkQueuePresentKHR(queue, reinterpret_cast<const VkPresentInfoKHR*>(&presentInfo)));
    return result;
}

std::string VulkanUtils::random_string(size_t length)
{
    {
        auto randchar = []() -> char {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randchar);
        return str;
    }
}

int VulkanUtils::random_int(int min, int max)
{
    return min + (rand() % static_cast<int>(max - min + 1));
}

vk::DeviceSize VulkanUtils::GetVulkanFormatSize(vk::Format format)
{
    switch(format)
    {
        case vk::Format::eR8Unorm:
        case vk::Format::eR8Snorm:
        case vk::Format::eR8Uint:
        case vk::Format::eR8Sint:
        case vk::Format::eR8Srgb:
            return 1;
        case vk::Format::eR16Unorm:
        case vk::Format::eR16Snorm:
        case vk::Format::eR16Uint:
        case vk::Format::eR16Sint:
        case vk::Format::eR16Sfloat:
            return 2;
        case vk::Format::eR32Uint:
        case vk::Format::eR32Sint:
        case vk::Format::eR32Sfloat:
            return 4;
        case vk::Format::eR8G8Unorm:
        case vk::Format::eR8G8Snorm:
        case vk::Format::eR8G8Uint:
        case vk::Format::eR8G8Sint:
        case vk::Format::eR8G8Srgb:
            return 2;
        case vk::Format::eR16G16Unorm:
        case vk::Format::eR16G16Snorm:
        case vk::Format::eR16G16Uint:
        case vk::Format::eR16G16Sint:
        case vk::Format::eR16G16Sfloat:
            return 4;
        case vk::Format::eR32G32Uint:
        case vk::Format::eR32G32Sint:
        case vk::Format::eR32G32Sfloat:
            return 8;
        case vk::Format::eR8G8B8Unorm:
        case vk::Format::eR8G8B8Snorm:
        case vk::Format::eR8G8B8Uint:
        case vk::Format::eR8G8B8Sint:
        case vk::Format::eR8G8B8Srgb:
            return 3;
        case vk::Format::eB8G8R8Unorm:
        case vk::Format::eB8G8R8Snorm:
        case vk::Format::eB8G8R8Uint:
        case vk::Format::eB8G8R8Sint:
        case vk::Format::eB8G8R8Srgb:
            return 3;
        case vk::Format::eR16G16B16Unorm:
        case vk::Format::eR16G16B16Snorm:
        case vk::Format::eR16G16B16Uint:
        case vk::Format::eR16G16B16Sint:
        case vk::Format::eR16G16B16Sfloat:
            return 6;
        case vk::Format::eR32G32B32Uint:
        case vk::Format::eR32G32B32Sint:
        case vk::Format::eR32G32B32Sfloat:
            return 12;
        case vk::Format::eR8G8B8A8Unorm:
        case vk::Format::eR8G8B8A8Snorm:
        case vk::Format::eR8G8B8A8Uint:
        case vk::Format::eR8G8B8A8Sint:
        case vk::Format::eR8G8B8A8Srgb:
            return 4;
        case vk::Format::eB8G8R8A8Unorm:
        case vk::Format::eB8G8R8A8Snorm:
        case vk::Format::eB8G8R8A8Uint:
        case vk::Format::eB8G8R8A8Sint:
        case vk::Format::eB8G8R8A8Srgb:
            return 4;
        case vk::Format::eR16G16B16A16Unorm:
        case vk::Format::eR16G16B16A16Snorm:
        case vk::Format::eR16G16B16A16Uint:
        case vk::Format::eR16G16B16A16Sint:
        case vk::Format::eR16G16B16A16Sfloat:
            return 8;
        case vk::Format::eR32G32B32A32Uint:
        case vk::Format::eR32G32B32A32Sint:
        case vk::Format::eR32G32B32A32Sfloat:
            return 16;
        // Depth formats
        case vk::Format::eD16Unorm:
            return 2;
        case vk::Format::eX8D24UnormPack32:
        case vk::Format::eD32Sfloat:
            return 4;
        case vk::Format::eD16UnormS8Uint:
            return 3;
        case vk::Format::eD24UnormS8Uint:
            return 4;
        case vk::Format::eD32SfloatS8Uint:
            return 5;
        default:
            throw std::runtime_error("Unsupported format size calculation");
    }
}

VulkanStructs::VStagingBufferInfo VulkanUtils::CreateStagingBuffer(const VulkanCore::VDevice& m_device, vk::DeviceSize size)
{

    std::string allocationNme = "Allocation of staging buffer for vertex, index or image ";

    VulkanStructs::VStagingBufferInfo staginBufferInfo = {};
    staginBufferInfo.size                             = size;

    VkBufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.size               = size;
    stagingBufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    std::vector<uint32_t> sharedQueueFamilyIndices = {//
                                                      m_device.GetQueueFamilyIndices().graphicsFamily.value().second,
                                                      m_device.GetQueueFamilyIndices().transferFamily.value().second};

    stagingBufferCreateInfo.queueFamilyIndexCount = sharedQueueFamilyIndices.size();
    stagingBufferCreateInfo.pQueueFamilyIndices   = sharedQueueFamilyIndices.data();

    VmaAllocationCreateInfo stagingAllocationCreateInfo = {};
    stagingAllocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    stagingAllocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    stagingAllocationCreateInfo.priority                = 1.0f;

    Utils::Logger::LogInfoVerboseOnly("Creating staging buffer...");
    assert(vmaCreateBuffer(m_device.GetAllocator(), &stagingBufferCreateInfo, &stagingAllocationCreateInfo,
                           &staginBufferInfo.m_stagingBufferVMA, &staginBufferInfo.m_stagingAllocation, nullptr)
           == VK_SUCCESS);
    staginBufferInfo.m_stagingBufferVK = staginBufferInfo.m_stagingBufferVMA;

    vmaSetAllocationName(m_device.GetAllocator(), staginBufferInfo.m_stagingAllocation, allocationNme.c_str());

    Utils::Logger::LogSuccess("Staging buffer created || SIZE: " + std::to_string(size) + "bytes ||");

    vmaMapMemory(m_device.GetAllocator(), staginBufferInfo.m_stagingAllocation, &staginBufferInfo.mappedPointer);

    return staginBufferInfo;
}

bool VulkanUtils::IsInViewFrustum(VulkanStructs::VBounds* bounds, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{

    glm::mat4 mvpMatrix = projection * view * model;

    glm::vec3 min = {1.5f, 1.5f, 1.5f};
    glm::vec3 max = {-1.5, -1.5, -1.5};

    // iterate through corners
    for(auto& corner : bounds->corners)
    {
        // project corner to clip space
        glm::vec4 projectedCorner = mvpMatrix * glm::vec4(bounds->origin + (corner * bounds->extents), 1.0f);

        // perspective devision
        projectedCorner.x /= projectedCorner.w;
        projectedCorner.y /= projectedCorner.w;
        projectedCorner.z /= projectedCorner.w;

        min = glm::min(glm::vec3(projectedCorner.x, projectedCorner.y, projectedCorner.z), min);
        max = glm::max(glm::vec3(projectedCorner.x, projectedCorner.y, projectedCorner.z), max);
    }

    // check the clip space box is within the view
    if(min.z > 1.f || max.z < 0.f || min.x > 1.f || max.x < -1.f || min.y > 1.f || max.y < -1.f)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int VulkanUtils::vkSampleToInt(vk::SampleCountFlagBits sample)
{
    switch(sample)
    {
        case vk::SampleCountFlagBits::e1:
            return 1;
        case vk::SampleCountFlagBits::e2:
            return 2;
        case vk::SampleCountFlagBits::e4:
            return 4;
        case vk::SampleCountFlagBits::e8:
            return 8;
        case vk::SampleCountFlagBits::e16:
            return 16;
        case vk::SampleCountFlagBits::e32:
            return 32;
        case vk::SampleCountFlagBits::e64:
            return 64;
        default:
            return 1;  // Fallback to 1 sample (safe default)
    }
}

vk::SampleCountFlagBits VulkanUtils::IntToVkSample(int sampleCount)
{
    switch(sampleCount)
    {
        case 1:
            return vk::SampleCountFlagBits::e1;
        case 2:
            return vk::SampleCountFlagBits::e2;
        case 4:
            return vk::SampleCountFlagBits::e4;
        case 8:
            return vk::SampleCountFlagBits::e8;
        case 16:
            return vk::SampleCountFlagBits::e16;
        case 32:
            return vk::SampleCountFlagBits::e32;
        case 64:
            return vk::SampleCountFlagBits::e64;
        default:
            return vk::SampleCountFlagBits::e1;  // Fallback to 1 sample (safe default)
    }
}
std::vector<char> VulkanUtils::ReadSPIRVShader(std::filesystem::path shaderPath)
{
    std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

    if(!file.is_open())
    {
        const auto err = "Failed to open SPIRV shader file at path: " + shaderPath.string()
                         + " did you compile the shaders using compile.sh script ?";
        throw std::runtime_error(err);
    }

    //create buffer to hold the binary
    size_t            fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    //go back to the begining and read the file again to get the content
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    assert(buffer.size() == fileSize);
    return buffer;
}
vk::ShaderModule VulkanUtils::CreateShaderModule(const VulkanCore::VDevice& device, const std::vector<char>& data)
{
    vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo;
    vertexShaderModuleCreateInfo.codeSize = data.size();
    vertexShaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(data.data());
    vertexShaderModuleCreateInfo.pNext    = nullptr;
    auto module = device.GetDevice().createShaderModule(vertexShaderModuleCreateInfo, nullptr);
    assert(module);
    Utils::Logger::LogInfoVerboseOnly("Created shader module");
    return module;
}
void VulkanUtils::Check(vk::Result result, vk::Result expectedResult)
{
    if(GlobalState::InDebugMode)
    {
        assert(result == expectedResult);
    }
    else
    {
        if(result != expectedResult)
            throw std::runtime_error("Result of the vulknan operation vas VK_FALSE which means that something went wrong, try restarting the application ");
    }
}

vk::DescriptorPool VulkanUtils::CreatePool(const VulkanCore::VDevice&                         devic,
                                           const VulkanCore::VDescriptorAllocator::PoolSizes& poolSizes,
                                           int                                                count,
                                           vk::DescriptorPoolCreateFlags                      flags)
{
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(poolSizes.sizes.size());

    for(auto& sz : poolSizes.sizes)
    {
        sizes.push_back({sz.first, uint32_t(sz.second * count)});
    }

    vk::DescriptorPoolCreateInfo poolCI;
    poolCI.flags         = flags;
    poolCI.maxSets       = count;
    poolCI.poolSizeCount = uint32_t(sizes.size());
    poolCI.pPoolSizes    = sizes.data();

    vk::DescriptorPool descriptorPool = devic.GetDevice().createDescriptorPool(poolCI, nullptr);

    return descriptorPool;
}
std::string VulkanUtils::DescriptorTypeToString(vk::DescriptorType descriptorType)
{
    switch(descriptorType)
    {
        case vk::DescriptorType::eSampler:
            return "eSampler";
        case vk::DescriptorType::eCombinedImageSampler:
            return "eCombinedImageSampler";
        case vk::DescriptorType::eSampledImage:
            return "eSampledImage";
        case vk::DescriptorType::eStorageImage:
            return "eStorageImage";
        case vk::DescriptorType::eUniformTexelBuffer:
            return "eUniformTexelBuffer";
        case vk::DescriptorType::eStorageTexelBuffer:
            return "eStorageTexelBuffer";
        case vk::DescriptorType::eUniformBuffer:
            return "eUniformBuffer";
        case vk::DescriptorType::eStorageBuffer:
            return "eStorageBuffer";
        case vk::DescriptorType::eUniformBufferDynamic:
            return "eUniformBufferDynamic";
        case vk::DescriptorType::eStorageBufferDynamic:
            return "eStorageBufferDynamic";
        case vk::DescriptorType::eInputAttachment:
            return "eInputAttachment";
        case vk::DescriptorType::eInlineUniformBlock:
            return "eInlineUniformBlock";
        case vk::DescriptorType::eAccelerationStructureKHR:
            return "eAccelerationStructureKHR";
        case vk::DescriptorType::eAccelerationStructureNV:
            return "eAccelerationStructureNV";
        default:
            return "Unknown";
    }
}

/**
 * Assert that does not kill the program but only prints out error message
 * @param condition condition for the assert
 * @param msg message ot be displayed
 * @return false if normal assert would be hit, return ture if condition is true
 */
bool VulkanUtils::RelaxedAssert(bool condition, std::string msg) {
    if (condition != true) {
        Utils::Logger::LogError(msg);
        return false;
    }return true;
}

