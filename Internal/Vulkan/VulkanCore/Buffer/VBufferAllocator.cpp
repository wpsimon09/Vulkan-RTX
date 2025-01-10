//
// Created by wpsimon09 on 07/01/25.
//

#include "VBufferAllocator.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VBufferAllocator::VBufferAllocator(const VulkanCore::VDevice& device):m_device(device)
    {
        //==============================
        // CREATE INITIAL VERTEX BUFFER
        // BB vertex
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");


        //==============================
        // CREATE INITIAL INDEX BUFFER
        // BB index
        //==============================

    }

    void VBufferAllocator::CreateBuffer(BufferAllocationInfo& allocationInfo)
    {
        VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = allocationInfo.size;
        bufferCreateInfo.usage = allocationInfo.usageFlags;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        std::vector<uint32_t> sharedQueueFamilyIndices = {
            m_device.GetQueueFamilyIndices().graphicsFamily.value().second,
            m_device.GetQueueFamilyIndices().transferFamily.value().second
        };

        bufferCreateInfo.queueFamilyIndexCount = sharedQueueFamilyIndices.size();
        bufferCreateInfo.pQueueFamilyIndices   = sharedQueueFamilyIndices.data();


        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        assert(vmaCreateBuffer(m_device.GetAllocator(),&bufferCreateInfo,&allocationCreateInfo, &allocationInfo.bufferVMA, &allocationInfo.allocationVMA,nullptr) == VK_SUCCESS);

        vmaSetAllocationName(m_device.GetAllocator(), allocationInfo.allocationVMA, "");
        Utils::Logger::LogSuccess("Buffer allocated successfully || SIZE: "+ std::to_string(size) + " bytes || ");
    }
} // VulkanCore