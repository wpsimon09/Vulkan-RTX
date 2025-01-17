//
// Created by wpsimon09 on 27/09/24.
//

#include "VGeneralUtils.hpp"
#include "stb_image/stb_image.h"

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

uint32_t VulkanUtils::FindQueueFamily(const std::vector<vk::QueueFamilyProperties> &queueFamilyProperties,
                                      vk::QueueFlagBits queueType) {
    //select just the queue fmily index that supports graphics operations
    std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty = std::find_if(
        queueFamilyProperties.begin(),
        queueFamilyProperties.end(),
        [queueType]( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & queueType; } );

    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    auto queueFamilyIndex =  static_cast<uint32_t> (std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    Utils::Logger::LogInfoVerboseOnly("Found graphics queue family at index: " + std::to_string(queueFamilyIndex));
    return queueFamilyIndex;
}

vk::ImageView VulkanUtils::GenerateImageView(const vk::Device &logicalDevice, const vk::Image &image,
    uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags aspecFlags) {
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image = image;
    createInfo.format = format;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.subresourceRange.aspectMask = aspecFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView = logicalDevice.createImageView(createInfo);
    assert(imageView != VK_NULL_HANDLE);
    Utils::Logger::LogInfoVerboseOnly("2D Image view created [this message was generated by utility function]");
    return imageView;
}

void VulkanUtils::GetVertexBindingAndAttributeDescription(vk::VertexInputBindingDescription &bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> &attributeDescription) {
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ApplicationCore::Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;

    attributeDescription.resize(3);

    // FOR POSITION
    attributeDescription[0].binding = 0;
    attributeDescription[0].location = 0;
    attributeDescription[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescription[0].offset = offsetof(ApplicationCore::Vertex, position);

    // NORMALS
    attributeDescription[1].binding = 0;
    attributeDescription[1].location = 1;
    attributeDescription[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescription[1].offset = offsetof(ApplicationCore::Vertex, normal);

    // uv
    attributeDescription[2].binding = 0;
    attributeDescription[2].location = 2;
    attributeDescription[2].format = vk::Format::eR32G32Sfloat;
    attributeDescription[2].offset = offsetof(ApplicationCore::Vertex, uv);
}

void VulkanUtils::CopyBuffers(const VulkanCore::VDevice &device, const vk::Buffer &srcBuffer,
    const vk::Buffer &dstBuffer, vk::DeviceSize size) {
    auto cmdBuffer = VulkanCore::VCommandBuffer(device, device.GetTransferCommandPool());
    Utils::Logger::LogInfoVerboseOnly("Copying buffers...");

    cmdBuffer.BeginRecording();

    vk::BufferCopy bufferCopy{};
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = size;

    cmdBuffer.GetCommandBuffer().copyBuffer(srcBuffer, dstBuffer, bufferCopy);

    cmdBuffer.EndRecording();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer.GetCommandBuffer();

    assert(device.GetTransferQueue().submit(1, &submitInfo, nullptr) == vk::Result::eSuccess);
    Utils::Logger::LogSuccess("Buffer copy completed !");
}

VulkanStructs::ImageData VulkanUtils::LoadImage(const std::string &path) {

    VulkanStructs::ImageData imageData{};

    imageData.pixels = reinterpret_cast<uint32_t*>(stbi_load(path.c_str(), &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
    imageData.channels = 4;
    imageData.fileName = path;

    if (!imageData.pixels) {
        Utils::Logger::LogError("Failed to generate texture at path: \t" + path);
        Utils::Logger::LogInfo("Failing back to the default texture");

        imageData.pixels = reinterpret_cast<uint32_t*>(stbi_load("Resources/DefaultTexture.jpg", &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
        imageData.channels = 4;
        imageData.fileName = path;

        if (!imageData.pixels) {
            throw std::runtime_error("Fallback to default texture failed, this should never happen !");
        }
    }else {
        Utils::Logger::LogSuccess("Image at path:\t" + path + "\n read successfully");
    }
      //-> to test the concurrency uncomment this line
    //std::this_thread::sleep_for(std::chrono::seconds(7));

    return imageData;

}

VulkanStructs::ImageData VulkanUtils::LoadImage(const TextureBufferInfo& data, const std::string& textureID)
{
    VulkanStructs::ImageData imageData{};

    if (
        (imageData.pixels = reinterpret_cast<uint32_t*>(stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(data.data), static_cast<int>(data.size), &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha))))
    {
        imageData.channels = 4;
        imageData.fileName = textureID;
    }

    else {

        Utils::Logger::LogError("Failed to generate texture that was loaded from memory, textureID was:" + textureID);
        Utils::Logger::LogInfo("Failing back to the default texture");

        imageData.pixels = reinterpret_cast<uint32_t*>(stbi_load("Resources/DefaultTexture.jpg", &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
        imageData.channels = 4;
        imageData.fileName = "Resources/DefaultTexture.jpg";

        if (!imageData.pixels) {
            throw std::runtime_error("Fallback to default texture failed, this should never happen !");
        }
    }

    //-> to test the concurrency uncomment this line
    //std::this_thread::sleep_for(std::chrono::seconds(7));

    return imageData;
}

std::string VulkanUtils::BufferUsageFlagToString(vk::BufferUsageFlags usage)
{
    std::string result;

    if (usage & vk::BufferUsageFlagBits::eTransferSrc) {
        result += "Transfer Source | ";
    }
    if (usage & vk::BufferUsageFlagBits::eTransferDst) {
        result += "Transfer Destination | ";
    }
    if (usage & vk::BufferUsageFlagBits::eUniformTexelBuffer) {
        result += "Uniform Texel Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eStorageTexelBuffer) {
        result += "Storage Texel Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eUniformBuffer) {
        result += "Uniform Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eStorageBuffer) {
        result += "Storage Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eIndexBuffer) {
        result += "Index Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eVertexBuffer) {
        result += "Vertex Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eIndirectBuffer) {
        result += "Indirect Buffer | ";
    }
    if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        result += "Shader Device Address | ";
    }
    if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddressEXT) {
        result += "Shader Device Address EXT | ";
    }
    if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddressKHR) {
        result += "Shader Device Address KHR | ";
    }
    if (usage & vk::BufferUsageFlagBits::eVideoDecodeSrcKHR) {
        result += "Video Decode Source KHR | ";
    }
    if (usage & vk::BufferUsageFlagBits::eVideoDecodeDstKHR) {
        result += "Video Decode Destination KHR | ";
    }
    if (usage & vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT) {
        result += "Transform Feedback Buffer EXT | ";
    }
    if (usage & vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT) {
        result += "Transform Feedback Counter Buffer EXT | ";
    }
    if (usage & vk::BufferUsageFlagBits::eConditionalRenderingEXT) {
        result += "Conditional Rendering EXT | ";
    }

    if (!result.empty()) {
        result.erase(result.size() - 3);
    } else {
        result = "Unknown Buffer Usage";
    }

    return result;
}

glm::mat4 VulkanUtils::FastGLTFToGLMMat4(fastgltf::math::fmat4x4& matrix)
{
    glm::mat4 newMatrix;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            newMatrix[row][col] = matrix[row][col];
    return newMatrix;
}

std::pair<vk::Result, uint32_t> VulkanUtils::SwapChainNextImageKHRWrapper(const VulkanCore::VDevice &device,
                                                                          const VulkanCore::VSwapChain &swapChain, uint64_t timeOut, const VulkanCore::VSyncPrimitive<vk::Semaphore>& semaphore,
                                                                          VulkanCore::VSyncPrimitive<vk::Fence> *fence) {
    uint32_t image_index;
    auto result = static_cast<vk::Result>(vkAcquireNextImageKHR(
            device.GetDevice(),
            swapChain.GetSwapChain(),
            timeOut,
            semaphore.GetSyncPrimitive(),
            nullptr,
            &image_index
        ));
    return std::make_pair(result, image_index);
}

vk::Result VulkanUtils::PresentQueueWrapper(vk::Queue queue, const vk::PresentInfoKHR &presentInfo) {
    auto result = static_cast<vk::Result>(vkQueuePresentKHR(queue,reinterpret_cast<const VkPresentInfoKHR*>(&presentInfo)));
    return result;
}

std::string VulkanUtils::random_string(size_t length)
{
    {
        auto randchar = []() -> char
        {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str;
    }
}

VulkanStructs::Bounds VulkanUtils::CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices)
{
    //========================
    // CALCULATE BOUNDING BOX
    //========================
    glm::vec3 maxPos = vertices[0].position;
    glm::vec3 minPos = vertices[0].position;

    for (const auto & i : vertices)
    {
        minPos = glm::min(minPos, i.position);
        maxPos = glm::max(maxPos, i.position);
    }
    VulkanStructs::Bounds bounds = {};
    bounds.origin = (maxPos + minPos) /2.f;
    bounds.extents = (maxPos - minPos) /2.f;
    bounds.max = maxPos;
    bounds.min = minPos;
    bounds.radius = glm::length(bounds.extents);


    return bounds;
}

VulkanStructs::StagingBufferInfo VulkanUtils::CreateStagingBuffer(const VulkanCore::VDevice& m_device,
    vk::DeviceSize size)
{

    std::string allocationNme = "Allocation of staging buffer for vertex, index or image ";

    VulkanStructs::StagingBufferInfo staginBufferInfo = {};
    staginBufferInfo.size = size;

    VkBufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.size = size;
    stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    std::vector<uint32_t> sharedQueueFamilyIndices = {
        //
        m_device.GetQueueFamilyIndices().graphicsFamily.value().second,
        m_device.GetQueueFamilyIndices().transferFamily.value().second
    };

    stagingBufferCreateInfo.queueFamilyIndexCount = sharedQueueFamilyIndices.size();
    stagingBufferCreateInfo.pQueueFamilyIndices = sharedQueueFamilyIndices.data();

    VmaAllocationCreateInfo stagingAllocationCreateInfo = {};
    stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingAllocationCreateInfo.flags =  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT ;
    stagingAllocationCreateInfo.priority = 1.0f;

    Utils::Logger::LogInfoVerboseOnly("Creating staging buffer...");
    assert(vmaCreateBuffer(m_device.GetAllocator(),&stagingBufferCreateInfo, &stagingAllocationCreateInfo, &staginBufferInfo.m_stagingBufferVMA, &staginBufferInfo.m_stagingAllocation,nullptr) == VK_SUCCESS);
    staginBufferInfo.m_stagingBufferVK = staginBufferInfo.m_stagingBufferVMA;

    vmaSetAllocationName(m_device.GetAllocator(), staginBufferInfo.m_stagingAllocation, allocationNme.c_str());

    Utils::Logger::LogSuccess("Staging buffer created || SIZE: " + std::to_string(size) + "bytes ||");

    vmaMapMemory(m_device.GetAllocator(), staginBufferInfo.m_stagingAllocation, &staginBufferInfo.mappedPointer);

    return staginBufferInfo;

}

bool VulkanUtils::IsInViewFrustum(VulkanStructs::Bounds* bounds, const glm::mat4& model, const glm::mat4& view,
    const glm::mat4& projection)
{

    glm::mat4 mvpMatrix = projection * view * model;

    glm::vec3 min = {1.5f, 1.5f, 1.5f};
    glm::vec3 max = { -1.5, -1.5, -1.5 };

    // iterate through corners
    for (auto& corner : bounds->corners)
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
    if (min.z > 1.f || max.z < 0.f || min.x > 1.f || max.x < -1.f || min.y > 1.f || max.y < -1.f) {
        return false;
    } else {
        return true;
    }
}


