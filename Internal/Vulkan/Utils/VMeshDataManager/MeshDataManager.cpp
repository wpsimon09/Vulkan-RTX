//
// Created by wpsimon09 on 07/01/25.
//

#include "MeshDataManager.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"


namespace VulkanCore {
    MeshDatatManager::MeshDatatManager(const VulkanCore::VDevice& device):m_device(device), m_indexBuffer_BB{}
    {
        m_transferCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);
        m_transferCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCommandPool);


        CreateNewVertexBuffers();
        CreateNewIndexBuffers();

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

        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");

        m_indexBuffer_BB.ID = 0;
        m_indexBuffer_BB.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        m_indexBuffer_BB.size = sizeof(uint32_t) * Indices_BB.size();
        CreateBuffer(m_indexBuffer_BB);

        auto stagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, Indices_BB.size() *sizeof(uint32_t));

        memcpy(stagingBuffer.mappedPointer, Indices_BB.data(), Indices_BB.size() * sizeof(uint32_t));
        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
        VulkanUtils::CopyBuffers(m_device, stagingBuffer.m_stagingBufferVK, m_indexBuffer_BB.bufferVK, Indices_BB.size() * sizeof(uint32_t));

        m_device.GetDevice().waitIdle();
        vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
    }

    VulkanStructs::MeshData MeshDatatManager::AddMeshData(std::vector<ApplicationCore::Vertex>& vertices,
        std::vector<uint32_t>& indices)
    {

        m_stagingVertices.insert(m_stagingVertices.end(), std::make_move_iterator(vertices.begin()), std::make_move_iterator(vertices.end()));
        m_stagingIndices.insert(m_stagingIndices.end(), indices.begin(), indices.end());

        auto bounds = VulkanUtils::CalculateBounds(vertices);
        VulkanStructs::MeshData meshData ={};

        meshData.vertexData = GenerateVertexBuffer(vertices);
        meshData.indexData = GenerateIndexBuffer(indices);
        meshData.vertexData_BB = GenerateVertexBuffer_BB(bounds);

        meshData.indexData_BB.buffer = m_indexBuffer_BB.bufferVK;
        meshData.indexData_BB.size = m_indexBuffer_BB.size;
        meshData.indexData_BB.offset = 0;
        meshData.bounds = bounds;

        return meshData;
    }

    VulkanStructs::BufferInfo MeshDatatManager::GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices)
    {
        VulkanStructs::BufferInfo bufferInfo = {
            .size = vertices.size() * sizeof(ApplicationCore::Vertex),
            .offset = m_currentVertexBuffer->currentOffset,
            .buffer = m_currentVertexBuffer->bufferVK,
        };
        m_currentVertexBuffer->currentOffset += vertices.size() * sizeof(ApplicationCore::Vertex);

        return bufferInfo;
    }

    VulkanStructs::BufferInfo MeshDatatManager::GenerateVertexBuffer_BB(
        VulkanStructs::Bounds& bounds)
    {
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

        m_stagingVertices_BB.insert(m_stagingVertices_BB.end(), std::make_move_iterator(Vertices_BB.begin()), std::make_move_iterator(Vertices_BB.end()));

        VulkanStructs::BufferInfo bufferInfo = {
            .size = Vertices_BB.size() * sizeof(ApplicationCore::Vertex),
            .offset = m_currentVertexBuffer_BB->currentOffset,
            .buffer = m_currentVertexBuffer_BB->bufferVK,
        };
        m_currentVertexBuffer_BB->currentOffset += Vertices_BB.size() * sizeof(ApplicationCore::Vertex);

        return bufferInfo;
    }

    VulkanStructs::BufferInfo MeshDatatManager::GenerateIndexBuffer(const std::vector<uint32_t>& indices)
    {
        VulkanStructs::BufferInfo bufferInfo = {
            .size = indices.size() * sizeof(uint32_t),
            .offset = m_currentIndexBuffer->currentOffset,
            .buffer = m_currentIndexBuffer->bufferVK,
        };

        m_currentIndexBuffer->currentOffset += indices.size() * sizeof(uint32_t);
        return bufferInfo;
    }

    void MeshDatatManager::UpdateGPU(vk::Semaphore semaphore)
    {
        //=========================================================================================================================================
        // VERTEX STAGIN BUFFER
        //==========================================================================================================================================
        auto vertexStaginBuffer = VulkanUtils::CreateStagingBuffer(m_device, m_stagingVertices.size() * sizeof(ApplicationCore::Vertex));
        memcpy(vertexStaginBuffer.mappedPointer, m_stagingVertices.data(), m_stagingVertices.size() * sizeof(ApplicationCore::Vertex));
        vmaUnmapMemory(m_device.GetAllocator(), vertexStaginBuffer.m_stagingAllocation);



        //=========================================================================================================================================
        // INDEX STAGINg BUFFER
        //==========================================================================================================================================
        auto indexStagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, m_stagingIndices.size() * sizeof(uint32_t));
        memcpy(indexStagingBuffer.mappedPointer, m_stagingIndices.data(), m_stagingIndices.size() * sizeof(uint32_t));
        vmaUnmapMemory(m_device.GetAllocator(), indexStagingBuffer.m_stagingAllocation);

        m_transferCommandBuffer->BeginRecording();

        // COPY VERTEX DATA TO THE GPU BUFFER
        {
            Utils::Logger::LogInfoVerboseOnly("Copying VERTEX buffer...");

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = m_currentVertexBuffer->copyOffSet;
            bufferCopy.size = vertexStaginBuffer.size;

            m_transferCommandBuffer->GetCommandBuffer().copyBuffer(vertexStaginBuffer.m_stagingBufferVK, m_currentVertexBuffer->bufferVK, bufferCopy);
        }

        //COPY INDEX DATA TO THE GPU
        {
            Utils::Logger::LogInfoVerboseOnly("Copying INDEX buffer...");

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = m_currentIndexBuffer->copyOffSet;
            bufferCopy.size = indexStagingBuffer.size;

            m_transferCommandBuffer->GetCommandBuffer().copyBuffer(indexStagingBuffer.m_stagingBufferVK, m_currentIndexBuffer->bufferVK, bufferCopy);

        }
        m_transferCommandBuffer->EndAndFlush(m_device.GetTransferQueue());

        m_device.GetDevice().waitIdle();
        m_currentVertexBuffer->copyOffSet += vertexStaginBuffer.size;
        m_currentIndexBuffer-> copyOffSet  += indexStagingBuffer.size;

        Utils::Logger::LogSuccess("Buffer copy of " +std::to_string(indexStagingBuffer.size + vertexStaginBuffer.size) + " bytes to vertex and index buffer  completed !");
        // CLEAN UP ONCE ALL DATA ARE IN GPU
        {
            vmaDestroyBuffer(m_device.GetAllocator(), indexStagingBuffer.m_stagingBufferVMA, indexStagingBuffer.m_stagingAllocation);
            vmaDestroyBuffer(m_device.GetAllocator(), vertexStaginBuffer.m_stagingBufferVMA, vertexStaginBuffer.m_stagingAllocation);

            m_stagingVertices.clear();
            m_stagingIndices.clear();
        }

    }


    void MeshDatatManager::Destroy()
    {
        for (int i = 0; i<m_vertexBuffers.size(); i++)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), m_vertexBuffers[i].bufferVMA, m_vertexBuffers[i].allocationVMA);
        }
        for (int i = 0; i<m_indexBuffers.size(); i++)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), m_indexBuffers[i].bufferVMA, m_indexBuffers[i].allocationVMA);
        }
        for (int i = 0; i<m_vertexBuffers_BB.size(); i++)
        {
            vmaDestroyBuffer(m_device.GetAllocator(), m_vertexBuffers_BB[i].bufferVMA, m_vertexBuffers_BB[i].allocationVMA);
        }

        vmaDestroyBuffer(m_device.GetAllocator(), m_indexBuffer_BB.bufferVMA, m_indexBuffer_BB.allocationVMA);

        m_transferCommandPool->Destroy();
    }

    void MeshDatatManager::CreateBuffer(VulkanStructs::GPUBufferInfo& allocationInfo) const
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



    void MeshDatatManager::CreateNewVertexBuffers()
    {
        //==============================
        // CREATE INITIAL VERTEX BUFFER
        // BB vertex
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::GPUBufferInfo newVertexBuffer{};
        newVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newVertexBuffer);
        m_vertexBuffers.emplace_back(newVertexBuffer);
        m_currentVertexBuffer = &m_vertexBuffers.back();
        m_currentVertexBuffer->ID = m_vertexBuffers.size();

        VulkanStructs::GPUBufferInfo newBBVertexBuffer{};
        newBBVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newBBVertexBuffer);
        m_vertexBuffers_BB.emplace_back(newBBVertexBuffer);
        m_currentVertexBuffer_BB = &m_vertexBuffers_BB.back();
        m_currentVertexBuffer_BB->ID = m_vertexBuffers_BB.size();
    }

    void MeshDatatManager::CreateNewIndexBuffers()
    {
        //==============================
        // CREATE INITIAL INDEX BUFFER
        // BB index
        //==============================
        Utils::Logger::LogInfo("Allocating NEW 16MB IndexBuffer");
        VulkanStructs::GPUBufferInfo newIndexBuffer{};
        newIndexBuffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        CreateBuffer(newIndexBuffer);
        m_indexBuffers.emplace_back(newIndexBuffer);
        m_currentIndexBuffer = &m_indexBuffers.back();
        m_currentIndexBuffer->ID = m_indexBuffers.size();
    }

} // VulkanCore