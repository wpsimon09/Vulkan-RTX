//
// Created by wpsimon09 on 07/01/25.
//

#include "MeshDataManager.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Application/Utils/ApplicationUtils.hpp"
#include <limits>

#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"


namespace VulkanCore {
    MeshDatatManager::MeshDatatManager(const VulkanCore::VDevice& device,  VulkanUtils::VTransferOperationsManager& transferOpsManager):m_device(device),m_transferOpsManager(transferOpsManager), m_indexBuffer_BB{}
    {
        m_transferCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);
        m_transferCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCommandPool);


        CreateNewVertexBuffers(true);
        CreateNewIndexBuffers();

        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");

        auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

        m_indexBuffer_BB.ID = 0;
        m_indexBuffer_BB.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        m_indexBuffer_BB.size = sizeof(uint32_t) * ApplicationCore::MeshData::Indices_BB.size();
        CreateBuffer(m_indexBuffer_BB);

        auto stagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, ApplicationCore::MeshData::Indices_BB.size() *sizeof(uint32_t));

        memcpy(stagingBuffer.mappedPointer, ApplicationCore::MeshData::Indices_BB.data(), ApplicationCore::MeshData::Indices_BB.size() * sizeof(uint32_t));
        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
        VulkanUtils::CopyBuffers(m_device,*bufferCopiedFence, stagingBuffer.m_stagingBufferVK, m_indexBuffer_BB.bufferVK, ApplicationCore::MeshData::Indices_BB.size() * sizeof(uint32_t), 0, 0);
        bufferCopiedFence->WaitForFence();
        bufferCopiedFence.reset();
        vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
    }

    VulkanStructs::MeshData MeshDatatManager::AddMeshData(std::vector<ApplicationCore::Vertex>& vertices,
        std::vector<uint32_t>& indices)
    {

        m_stagingVertices.insert(m_stagingVertices.end(), std::make_move_iterator(vertices.begin()), std::make_move_iterator(vertices.end()));
        m_stagingIndices.insert(m_stagingIndices.end(), indices.begin(), indices.end());

        auto bounds = ApplicationCore::CalculateBounds(vertices);
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

    VulkanStructs::GPUSubBufferInfo MeshDatatManager::GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices)
    {
        SelectMostSuitableBuffer(EBufferType::Vertex, vertices.size()*sizeof(ApplicationCore::Vertex));

        VulkanStructs::GPUSubBufferInfo bufferInfo = {
            .size = vertices.size() * sizeof(ApplicationCore::Vertex),
            .offset = m_currentVertexBuffer->currentOffset,
            .buffer = m_currentVertexBuffer->bufferVK,
            .ID = VulkanUtils::random_int(1, std::numeric_limits<int>::max() - 1)
        };
        m_currentVertexBuffer->currentOffset += vertices.size() * sizeof(ApplicationCore::Vertex);

        return bufferInfo;
    }

    VulkanStructs::GPUSubBufferInfo MeshDatatManager::GenerateVertexBuffer_BB(
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

        m_stagingVertices_BB.insert(m_stagingVertices_BB.end(), Vertices_BB.begin(), Vertices_BB.end());

        VulkanStructs::GPUSubBufferInfo bufferInfo = {
            .size = Vertices_BB.size() * sizeof(ApplicationCore::Vertex),
            .offset = m_currentVertexBuffer_BB->currentOffset,
            .buffer = m_currentVertexBuffer_BB->bufferVK,
            .ID = VulkanUtils::random_int(1, std::numeric_limits<int>::max() - 1)
        };

        m_currentVertexBuffer_BB->currentOffset += Vertices_BB.size() * sizeof(ApplicationCore::Vertex);

        return bufferInfo;
    }

    VulkanStructs::GPUSubBufferInfo MeshDatatManager::GenerateIndexBuffer(const std::vector<uint32_t>& indices)
    {
        SelectMostSuitableBuffer(EBufferType::Index, indices.size()*sizeof(uint32_t));

        VulkanStructs::GPUSubBufferInfo bufferInfo = {
            .size = indices.size() * sizeof(uint32_t),
            .offset = m_currentIndexBuffer->currentOffset,
            .buffer = m_currentIndexBuffer->bufferVK,
            .ID = VulkanUtils::random_int(1, std::numeric_limits<int>::max() - 1)
        };

        m_currentIndexBuffer->currentOffset += indices.size() * sizeof(uint32_t);
        return bufferInfo;
    }

    void MeshDatatManager::UpdateGPU(vk::Semaphore semaphore)
    {
        //=========================================================================================================================================
        // VERTEX STAGING BUFFER
        //==========================================================================================================================================
        auto vertexStaginBuffer = VulkanUtils::CreateStagingBuffer(m_device, m_stagingVertices.size() * sizeof(ApplicationCore::Vertex));
        memcpy(vertexStaginBuffer.mappedPointer, m_stagingVertices.data(), vertexStaginBuffer.size);
        vmaUnmapMemory(m_device.GetAllocator(), vertexStaginBuffer.m_stagingAllocation);

        //=========================================================================================================================================
        // VERTEX_BB STAGING BUFFER
        //==========================================================================================================================================
        auto vertexStaginBuffer_BB = VulkanUtils::CreateStagingBuffer(m_device,  m_stagingVertices_BB.size() * sizeof(ApplicationCore::Vertex));
        memcpy(vertexStaginBuffer_BB.mappedPointer, m_stagingVertices_BB.data(), vertexStaginBuffer_BB.size);
        vmaUnmapMemory(m_device.GetAllocator(), vertexStaginBuffer_BB.m_stagingAllocation);


        //=========================================================================================================================================
        // INDEX STAGING BUFFER
        //==========================================================================================================================================
        auto indexStagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, m_stagingIndices.size() * sizeof(uint32_t));
        memcpy(indexStagingBuffer.mappedPointer, m_stagingIndices.data(), indexStagingBuffer.size);
        vmaUnmapMemory(m_device.GetAllocator(), indexStagingBuffer.m_stagingAllocation);


        assert(m_transferOpsManager.GetCommandBuffer().GetIsRecording() && "Command buffer is not recording any commands, before using it make sure it is in recording state  !");
        auto &cmdBuffer = m_transferOpsManager.GetCommandBuffer().GetCommandBuffer();

        // COPY VERTEX DATA TO THE GPU BUFFER
        {
            Utils::Logger::LogInfoVerboseOnly("Copying VERTEX buffer...");

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = m_currentVertexBuffer->copyOffSet;
            bufferCopy.size = vertexStaginBuffer.size;

            cmdBuffer.copyBuffer(vertexStaginBuffer.m_stagingBufferVK, m_currentVertexBuffer->bufferVK, bufferCopy);
        }

        // COPY VERTEX BB DATA TO THE GPU BUFFER
        {
            Utils::Logger::LogInfoVerboseOnly("Copying VERTEX-BB buffer...");

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = m_currentVertexBuffer_BB->copyOffSet;
            bufferCopy.size = vertexStaginBuffer_BB.size;

            cmdBuffer.copyBuffer(vertexStaginBuffer_BB.m_stagingBufferVK, m_currentVertexBuffer_BB->bufferVK, bufferCopy);
        }

        //COPY INDEX DATA TO THE GPU
        {
            Utils::Logger::LogInfoVerboseOnly("Copying INDEX buffer...");

            vk::BufferCopy bufferCopy{};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = m_currentIndexBuffer->copyOffSet;
            bufferCopy.size = indexStagingBuffer.size;

            cmdBuffer.copyBuffer(indexStagingBuffer.m_stagingBufferVK, m_currentIndexBuffer->bufferVK, bufferCopy);

        }


        m_currentVertexBuffer->copyOffSet += vertexStaginBuffer.size;
        m_currentIndexBuffer-> copyOffSet  += indexStagingBuffer.size;
        m_currentVertexBuffer_BB->copyOffSet += vertexStaginBuffer_BB.size;
        Utils::Logger::LogSuccess("Buffer copy of " +std::to_string(indexStagingBuffer.size + vertexStaginBuffer.size) + " bytes to vertex and index buffer  completed !");
        // CLEAN UP ONCE ALL DATA ARE IN GPU
        {
            m_transferOpsManager.DestroyBuffer(indexStagingBuffer.m_stagingBufferVMA, indexStagingBuffer.m_stagingAllocation );
            m_transferOpsManager.DestroyBuffer(vertexStaginBuffer.m_stagingBufferVMA, vertexStaginBuffer.m_stagingAllocation );
            m_transferOpsManager.DestroyBuffer(vertexStaginBuffer_BB.m_stagingBufferVMA, vertexStaginBuffer_BB.m_stagingAllocation );

            m_stagingVertices.clear();
            m_stagingVertices_BB.clear();
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



    void MeshDatatManager::CreateNewVertexBuffers(bool createForBoundingBox)
    {
        m_vertexBuffers.reserve(BUFFER_SIZE);
        //==============================
        // CREATE INITIAL VERTEX BUFFER
        // BB vertex
        //==============================
        Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");
        VulkanStructs::GPUBufferInfo newVertexBuffer{};
        newVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
        CreateBuffer(newVertexBuffer);
        m_vertexBuffers.emplace_back(newVertexBuffer);
        m_currentVertexBuffer = &m_vertexBuffers.back();
        m_currentVertexBuffer->ID = static_cast<int>(m_vertexBuffers.size());

        if (createForBoundingBox)
        {
            VulkanStructs::GPUBufferInfo newBBVertexBuffer{};
            newBBVertexBuffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;;
            CreateBuffer(newBBVertexBuffer);
            m_vertexBuffers_BB.emplace_back(newBBVertexBuffer);
            m_currentVertexBuffer_BB = &m_vertexBuffers_BB.back();
            m_currentVertexBuffer_BB->ID = m_vertexBuffers_BB.size();
        }
    }

    void MeshDatatManager::CreateNewIndexBuffers()
    {
        //==============================
        // CREATE INITIAL INDEX BUFFER
        // BB index
        //==============================
        m_indexBuffers.reserve(BUFFER_SIZE);
        Utils::Logger::LogInfo("Allocating NEW 16MB IndexBuffer");
        VulkanStructs::GPUBufferInfo newIndexBuffer{};
        newIndexBuffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
        CreateBuffer(newIndexBuffer);
        m_indexBuffers.emplace_back(newIndexBuffer);
        m_currentIndexBuffer = &m_indexBuffers.back();
        m_currentIndexBuffer->ID = m_indexBuffers.size();
    }

    void MeshDatatManager::SelectMostSuitableBuffer(EBufferType bufferType, vk::DeviceSize subAllocationSize)
    {
        std::vector<VulkanStructs::GPUBufferInfo>& buffers = bufferType == EBufferType::Vertex ? m_vertexBuffers : m_indexBuffers;

        for (auto &buffer : buffers)
        {
            if (buffer.WillNewBufferFit(subAllocationSize))
            {
                Utils::Logger::LogInfoVerboseOnly("Buffer will fit the current chunk, skiping allocation...");
                if (bufferType == EBufferType::Vertex) m_currentVertexBuffer = &buffer;
                else  if(bufferType == EBufferType::Index) m_currentIndexBuffer = &buffer;
                return;
            }else
            {
                Utils::Logger::LogInfo("Buffer that attempts to be allocated will not fit the current buffer ! Allocating new 16MB buffer chunk");
                // function CreateNewVertexBuffer will assign right pointer to the m_CurretnBuffer
                if (bufferType == EBufferType::Vertex) CreateNewVertexBuffers(false);
                else if (bufferType == EBufferType::Index) CreateNewIndexBuffers();

            }
        }
    }

    std::vector<ApplicationCore::Vertex> MeshDatatManager::ReadBack(VulkanStructs::GPUSubBufferInfo& bufferInfo){
        auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);
        std::vector<ApplicationCore::Vertex> vertices;
        vertices.resize(bufferInfo.size / sizeof(ApplicationCore::Vertex));

        // create staging buffer to copy readback memory from
        auto stagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, bufferInfo.size);
        stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;

        VulkanUtils::CopyBuffers(m_device,*bufferCopiedFence, bufferInfo.buffer, stagingBuffer.m_stagingBufferVK, bufferInfo.size, bufferInfo.offset, 0);
        
        bufferCopiedFence->WaitForFence();
        memcpy(vertices.data(), stagingBuffer.mappedPointer, bufferInfo.size);

        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
        vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
        bufferCopiedFence->Destroy();

        return vertices;

    }

    std::vector<VulkanStructs::ReadBackBufferInfo<ApplicationCore::Vertex>> MeshDatatManager::ReadBackVertexBuffer(){

        auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

        std::vector<VulkanStructs::ReadBackBufferInfo<ApplicationCore::Vertex>> vertexReadBackBufferInfos;
        vertexReadBackBufferInfos.resize(m_vertexBuffers.size());

        // create staging buffer to copy readback memory from
        for(int i =0; i<m_vertexBuffers.size(); i++){

            vertexReadBackBufferInfos[i].bufferID = m_vertexBuffers[i].ID;
            vertexReadBackBufferInfos[i].size = m_vertexBuffers[i].size;
            vertexReadBackBufferInfos[i].data.resize(m_vertexBuffers[i].currentOffset / sizeof(ApplicationCore::Vertex));

            auto stagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, m_vertexBuffers[i].size);
            stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;
    
            VulkanUtils::CopyBuffers(m_device,*bufferCopiedFence, m_vertexBuffers[i].bufferVK, stagingBuffer.m_stagingBufferVK, m_vertexBuffers[i].currentOffset,0,0);
            bufferCopiedFence->WaitForFence();
            bufferCopiedFence->ResetFence();
            memcpy(vertexReadBackBufferInfos[i].data.data(), stagingBuffer.mappedPointer, m_vertexBuffers[i].currentOffset);
            
            vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
            vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
        }
        bufferCopiedFence->Destroy();
        return vertexReadBackBufferInfos;
    }

    std::vector<VulkanStructs::ReadBackBufferInfo<uint32_t>> MeshDatatManager::ReadBackIndexBuffers(){

        auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

        std::vector<VulkanStructs::ReadBackBufferInfo<uint32_t>> indexReadBackBufferInfos;
        indexReadBackBufferInfos.resize(m_indexBuffers.size());

        // create staging buffer to copy readback memory from
        for(int i =0; i<m_indexBuffers.size(); i++){

            indexReadBackBufferInfos[i].bufferID = m_indexBuffers[i].ID;
            indexReadBackBufferInfos[i].size = m_indexBuffers[i].size;
            indexReadBackBufferInfos[i].data.resize(m_indexBuffers[i].currentOffset / sizeof(uint32_t));

            auto stagingBuffer = VulkanUtils::CreateStagingBuffer(m_device, m_indexBuffers[i].size);
            stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;
    
            VulkanUtils::CopyBuffers(m_device, *bufferCopiedFence, m_indexBuffers[i].bufferVK, stagingBuffer.m_stagingBufferVK, m_indexBuffers[i].currentOffset,0,0 );
            bufferCopiedFence->WaitForFence();
            bufferCopiedFence->ResetFence();                                              
            memcpy(indexReadBackBufferInfos[i].data.data(), stagingBuffer.mappedPointer, m_indexBuffers[i].currentOffset);
            
            vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
            vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
        }
        bufferCopiedFence->Destroy();
        return indexReadBackBufferInfos;
    }
} // VulkanCore