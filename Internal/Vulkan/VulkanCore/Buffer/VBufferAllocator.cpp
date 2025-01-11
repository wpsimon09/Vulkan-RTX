//
// Created by wpsimon09 on 07/01/25.
//

#include "VBufferAllocator.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VBufferAllocator::VBufferAllocator(const VulkanCore::VDevice& device):m_device(device)
    {
        //==============================
        // CREATE INITIAL VERTEX BUFFER
        // BB vertex
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::BufferAllocationInfo newVertexBuffer{};
        newVertexBuffer.ID = 0;
        newVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newVertexBuffer);
        m_vertexBuffers.emplace_back(newVertexBuffer);

        VulkanStructs::BufferAllocationInfo newBBVertexBuffer{};
        newBBVertexBuffer.ID = 0;
        newBBVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newBBVertexBuffer);
        m_BBvertexBuffers.emplace_back(newBBVertexBuffer);



        //==============================
        // CREATE INITIAL INDEX BUFFER
        // BB index
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::BufferAllocationInfo newIndexBuffer{};
        newIndexBuffer.ID = 0;
        newIndexBuffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newIndexBuffer);
        m_indexBuffers.emplace_back(newIndexBuffer);

        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::BufferAllocationInfo newBBIndexBuffer{};
        newBBIndexBuffer.ID = 0;
        newBBIndexBuffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newBBIndexBuffer);
        m_indexBuffers.emplace_back(newBBIndexBuffer);

    }

    VulkanStructs::BufferInfo VBufferAllocator::AddVertexBuffer(std::vector<ApplicationCore::Vertex>& vertices)
    {
    }

    VulkanStructs::BufferInfo VBufferAllocator::AddIndexBuffer(std::vector<ApplicationCore::Vertex>& vertices)
    {

    }

    void VBufferAllocator::Destroy()
    {
        for (int i = 0; i<m_vertexBuffers.size(); i++)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), m_vertexBuffers[i].bufferVMA, m_vertexBuffers[i].allocationVMA);
        }for (int i = 0; i<m_indexBuffers.size(); i++)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), m_indexBuffers[i].bufferVMA, m_indexBuffers[i].allocationVMA);
        }for (int i = 0; i<m_BBvertexBuffers.size(); i++)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), m_BBvertexBuffers[i].bufferVMA, m_BBvertexBuffers[i].allocationVMA);
        }for (int i = 0; i<m_BBindexBuffers.size(); i++){
            vmaDestroyBuffer(m_device.GetAllocator(), m_BBindexBuffers[i].bufferVMA, m_BBindexBuffers[i].allocationVMA);
        }
    }

    void VBufferAllocator::CreateBuffer(VulkanStructs::BufferAllocationInfo& allocationInfo)
    {
        VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = allocationInfo.size;
        bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(allocationInfo.usageFlags);
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

        vmaSetAllocationName(m_device.GetAllocator(), allocationInfo.allocationVMA, VulkanUtils::BufferUsageFlagToString(allocationInfo.usageFlags).c_str());
        Utils::Logger::LogSuccess("Buffer allocated successfully || SIZE: "+ std::to_string(allocationInfo.size) + " bytes || ");

        allocationInfo.bufferVK = allocationInfo.bufferVMA;
    }

    void VBufferAllocator::CreateStagingBuffer(VkDeviceSize size) {

        std::string allocationNme = "Allocation of staging buffer for vertex, index or image ";

        VkBufferCreateInfo stagingBufferCreateInfo = {};
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.size = size;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        std::vector<uint32_t> sharedQueueFamilyIndices = {
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
        assert(vmaCreateBuffer(m_device.GetAllocator(),&stagingBufferCreateInfo, &stagingAllocationCreateInfo, &m_stagingBufferVMA, &m_stagingAllocation,nullptr) == VK_SUCCESS);
        m_stagingBufferVK = m_stagingBufferVMA;

        vmaSetAllocationName(m_device.GetAllocator(), m_stagingAllocation, allocationNme.c_str());

        Utils::Logger::LogSuccess("Staging buffer created || SIZE: " + std::to_string(size) + "bytes ||");
    }
} // VulkanCore