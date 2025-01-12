//
// Created by wpsimon09 on 07/01/25.
//

#include "VBufferAllocator.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"

namespace VulkanCore {
    VBufferAllocator::VBufferAllocator(const VulkanCore::VDevice& device):m_device(device)
    {
        m_transferCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);
        m_transferCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCommandPool);

        CreateNewVertexBuffers();
        CreateNewIndexBuffers();
    }

    VulkanStructs::BufferInfo VBufferAllocator::AddVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices)
    {

        //create new staging buffer
        m_stagingVertexBuffers.emplace_back(CreateStagingBuffer(vertices.size() * sizeof(ApplicationCore::Vertex)));
        auto& stagingBuffer = m_stagingVertexBuffers.back();

        //copy data to staging buffer and add it to the array
        memcpy(stagingBuffer.mappedPointer, vertices.data(), vertices.size() * sizeof(ApplicationCore::Vertex));

        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);

        stagingBuffer.copyDstBuffer = m_currentVertexBuffer->bufferVK;

        VulkanStructs::BufferInfo bufferInfo = {
            .size = vertices.size() * sizeof(ApplicationCore::Vertex),
            .offset = m_currentVertexBuffer->currentOffset,
            .buffer = m_currentVertexBuffer->bufferVK
        };

        m_currentVertexBuffer->currentOffset += vertices.size() * sizeof(ApplicationCore::Vertex);

        return bufferInfo;
    }

    VulkanStructs::BufferInfo VBufferAllocator::AddIndexBuffer(const std::vector<uint32_t>& indices)
    {

    }

    void VBufferAllocator::UpdateGPU(vk::Semaphore semaphore)
    {
        Utils::Logger::LogInfoVerboseOnly("Copying buffers...");
        m_transferCommandBuffer->BeginRecording();
        for (auto& stagingVertexBuffer : m_stagingVertexBuffers)
        {

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = stagingVertexBuffer.;
            bufferCopy.size = stagingVertexBuffer.size;

            cmdBuffer.GetCommandBuffer().copyBuffer(srcBuffer, dstBuffer, bufferCopy);

            cmdBuffer.EndRecording();

            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmdBuffer.GetCommandBuffer();

            assert(device.GetTransferQueue().submit(1, &submitInfo, nullptr) == vk::Result::eSuccess);
            Utils::Logger::LogSuccess("Buffer copy completed !");
        }
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

    VulkanStructs::StagingBufferAllocationInfo VBufferAllocator::CreateStagingBuffer(VkDeviceSize size) {

        std::string allocationNme = "Allocation of staging buffer for vertex, index or image ";

        VulkanStructs::StagingBufferAllocationInfo allocationInfo = {};
        allocationInfo.size = size;

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
        assert(vmaCreateBuffer(m_device.GetAllocator(),&stagingBufferCreateInfo, &stagingAllocationCreateInfo, &allocationInfo.m_stagingBufferVMA, &allocationInfo.m_stagingAllocation,nullptr) == VK_SUCCESS);
        allocationInfo.m_stagingBufferVK = allocationInfo.m_stagingBufferVMA;

        vmaSetAllocationName(m_device.GetAllocator(), allocationInfo.m_stagingAllocation, allocationNme.c_str());

        Utils::Logger::LogSuccess("Staging buffer created || SIZE: " + std::to_string(size) + "bytes ||");

        vmaMapMemory(m_device.GetAllocator(), allocationInfo.m_stagingAllocation, &allocationInfo.mappedPointer);

        return allocationInfo;
    }


    void VBufferAllocator::CreateNewVertexBuffers()
    {
        //==============================
        // CREATE INITIAL VERTEX BUFFER
        // BB vertex
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::BufferAllocationInfo newVertexBuffer{};
        newVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newVertexBuffer);
        m_vertexBuffers.emplace_back(newVertexBuffer);
        m_currentVertexBuffer = &m_vertexBuffers.back();
        m_currentVertexBuffer->ID = m_vertexBuffers.size();

        VulkanStructs::BufferAllocationInfo newBBVertexBuffer{};
        newBBVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newBBVertexBuffer);
        m_BBvertexBuffers.emplace_back(newBBVertexBuffer);
        m_currentBBvertexBuffer = &m_BBvertexBuffers.back();
        m_currentBBvertexBuffer->ID = m_BBvertexBuffers.size();
    }

    void VBufferAllocator::CreateNewIndexBuffers()
    {
        //==============================
        // CREATE INITIAL INDEX BUFFER
        // BB index
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::BufferAllocationInfo newIndexBuffer{};
        newIndexBuffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newIndexBuffer);
        m_indexBuffers.emplace_back(newIndexBuffer);
        m_currentIndexBuffer = &m_indexBuffers.back();
        m_currentIndexBuffer->ID = m_indexBuffers.size();

        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::BufferAllocationInfo newBBIndexBuffer{};
        newBBIndexBuffer.ID = 0;
        newBBIndexBuffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newBBIndexBuffer);
        m_BBindexBuffers.emplace_back(newBBIndexBuffer);
        m_currentBBindexBuffer = &m_BBindexBuffers.back();
        m_currentBBindexBuffer->ID = m_BBindexBuffers.size();
    }


} // VulkanCore