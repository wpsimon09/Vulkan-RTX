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
        stagingBuffer.dstOffset = m_currentVertexBuffer->currentOffset;

        VulkanStructs::BufferInfo bufferInfo = {
            .size = vertices.size() * sizeof(ApplicationCore::Vertex),
            .offset = m_currentVertexBuffer->currentOffset,
            .buffer = m_currentVertexBuffer->bufferVK,
            .bounds = CalculateBounds(vertices),
        };
        m_currentVertexBuffer->currentOffset += vertices.size() * sizeof(ApplicationCore::Vertex);

        return bufferInfo;
    }

    VulkanStructs::BufferInfo VBufferAllocator::AddIndexBuffer(const std::vector<uint32_t>& indices)
    {
        m_stagingIndexBuffers.emplace_back(CreateStagingBuffer(indices.size() * sizeof(uint32_t)));
        auto& stagingBuffer = m_stagingIndexBuffers.back();
        memcpy(stagingBuffer.mappedPointer, indices.data(), indices.size() * sizeof(uint32_t));
        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);

        stagingBuffer.copyDstBuffer = m_currentIndexBuffer->bufferVK;
        stagingBuffer.dstOffset = m_currentIndexBuffer->currentOffset;

        VulkanStructs::BufferInfo bufferInfo = {
            .size = indices.size() * sizeof(uint32_t),
            .offset = m_currentIndexBuffer->currentOffset,
            .buffer = m_currentIndexBuffer->bufferVK,
            .bounds = {}
        };

        m_currentIndexBuffer->currentOffset += indices.size() * sizeof(uint32_t);
        return bufferInfo;
    }

    void VBufferAllocator::UpdateGPU(vk::Semaphore semaphore)
    {
        Utils::Logger::LogInfoVerboseOnly("Copying buffers...");
        m_device.GetDevice().waitIdle();
        m_transferCommandBuffer->BeginRecording();
        for (auto& stagingVertexBuffer : m_stagingVertexBuffers)
        {

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = stagingVertexBuffer.dstOffset;
            bufferCopy.size = stagingVertexBuffer.size;

            m_transferCommandBuffer->GetCommandBuffer().copyBuffer(stagingVertexBuffer.m_stagingBufferVK, stagingVertexBuffer.copyDstBuffer, bufferCopy);
        }

        m_transferCommandBuffer->EndAndFlush(m_device.GetTransferQueue());
        ClearVertexStagingBuffers();
        Utils::Logger::LogSuccess("Buffer copy completed !");
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

        m_transferCommandPool->Destroy();
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

    void VBufferAllocator::ClearVertexStagingBuffers()
    {
        for (auto& stagingBuffer : m_stagingVertexBuffers)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA,stagingBuffer.m_stagingAllocation);
        }
        m_stagingVertexBuffers.clear();
    }

    VulkanStructs::Bounds VBufferAllocator::CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices)
    {
        //========================
        // CALCULATE BOUNDING BOX
        //========================
        glm::vec3 maxPos = vertices[0].position;
        glm::vec3 minPos = vertices[0].position;;

        for (const auto & i : vertices)
        {
            minPos = glm::min(minPos, i.position);
            maxPos = glm::max(maxPos, i.position);
        }
        VulkanStructs::Bounds bounds = {};
        bounds.origin = (maxPos + minPos) /2.f;
        bounds.extents = (maxPos - minPos) /2.f;
        bounds.radius = glm::length(bounds.extents);

        std::vector<ApplicationCore::Vertex> Vertices_BB = {
            {bounds.origin + glm::vec3(-bounds.extents.x, -bounds.extents.y, -bounds.extents.z), {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // V0
            {bounds.origin + glm::vec3(+bounds.extents.x, -bounds.extents.y, -bounds.extents.z), {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // V1
            {bounds.origin + glm::vec3(+bounds.extents.x, +bounds.extents.y, -bounds.extents.z), {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // V2
            {bounds.origin + glm::vec3(-bounds.extents.x, +bounds.extents.y, -bounds.extents.z), {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // V3
            {bounds.origin + glm::vec3(-bounds.extents.x, -bounds.extents.y, +bounds.extents.z), {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // V4
            {bounds.origin + glm::vec3(+bounds.extents.x, -bounds.extents.y, +bounds.extents.z), {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // V5
            {bounds.origin + glm::vec3(+bounds.extents.x, +bounds.extents.y, +bounds.extents.z), {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // V6
            {bounds.origin + glm::vec3(-bounds.extents.x, +bounds.extents.y, +bounds.extents.z), {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // V7
        };

        // used for topology line
        std::vector<uint32_t> Indices_BB = {
            // Bottom face
            0, 1, 2, 0, 2, 3,
            // Top face
            4, 5, 6, 4, 6, 7,
            // Front face
            0, 1, 5, 0, 5, 4,
            // Back face
            3, 2, 6, 3, 6, 7,
            // Left face
            0, 4, 7, 0, 7, 3,
            // Right face
            1, 5, 6, 1, 6, 2
        };

        //=======================================================
        // CREATE STAGING BUFFERS FOR BOUNDING BOX VISUALISATIONS
        //=======================================================

        //BB-Vertices
        m_stagingVertexBuffers.emplace_back(CreateStagingBuffer(Vertices_BB.size() * sizeof(ApplicationCore::Vertex)));
        auto& stagingBuffer = m_stagingVertexBuffers.back();

        //copy data to staging buffer and add it to the array
        memcpy(stagingBuffer.mappedPointer, Vertices_BB.data(), Vertices_BB.size() * sizeof(ApplicationCore::Vertex));

        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);

        stagingBuffer.copyDstBuffer = m_currentBBvertexBuffer->bufferVK;
        stagingBuffer.dstOffset = m_currentBBvertexBuffer->currentOffset;

        m_currentBBvertexBuffer->currentOffset += sizeof(ApplicationCore::Vertex) * Vertices_BB.size();

        //BB-indices
        m_stagingIndexBuffers.emplace_back(CreateStagingBuffer(Indices_BB.size() * sizeof(uint32_t)));
        stagingBuffer = m_stagingIndexBuffers.back();

        //copy data to staging buffer and add it to the array
        memcpy(stagingBuffer.mappedPointer, Indices_BB.data(), Indices_BB.size() * sizeof(uint32_t));

        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);

        stagingBuffer.copyDstBuffer = m_currentBBindexBuffer->bufferVK;
        stagingBuffer.dstOffset =  m_currentBBindexBuffer->currentOffset;

        m_currentBBindexBuffer->currentOffset += sizeof(uint32_t) * Indices_BB.size();

        bounds.BB_BufferVertex = m_currentBBvertexBuffer->bufferVK;
        bounds.BB_BufferIndex = m_currentBBindexBuffer->bufferVK;

        return bounds;
    }
} // VulkanCore