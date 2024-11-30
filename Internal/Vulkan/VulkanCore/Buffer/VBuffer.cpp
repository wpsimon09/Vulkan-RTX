//
// Created by wpsimon09 on 20/10/24.
//

#include "VBuffer.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "vulkan/vulkan.h"

namespace VulkanCore {

    VBuffer::VBuffer(const VDevice &device, const std::string& name): VObject(), m_device(device), m_allocationName(name) {
        m_sharedQueueFamilyIndices = {
            device.GetQueueFamilyIndices().graphicsFamily.value().second,
            device.GetQueueFamilyIndices().transferFamily.value().second
        };
        m_isInitialized = false;
        m_isPresistentlyMapped = false;
    }

    void * VBuffer::MapStagingBuffer() {
        Utils::Logger::LogInfo("Mapping staging buffer...");
        vmaMapMemory(m_device.GetAllocator(), m_stagingAllocation, &m_mappedData);
        return m_mappedData;
    }

    void VBuffer::UnMapStagingBuffer() {
        Utils::Logger::LogInfo("Unmapping staging buffer...");
        vmaUnmapMemory(m_device.GetAllocator() , m_stagingAllocation);
    }

    vk::DescriptorBufferInfo & VBuffer::GetBufferInfoForDescriptor() {
        assert(m_bufferType == vk::BufferUsageFlagBits::eUniformBuffer && "This buffer is not uniform buffer and therefore can not be used in descriptors");
        return m_descriptorBufferInfo;
    }

    void VBuffer::MakeVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices) {
        assert(!m_isInitialized);

        //---------------------
        // CREATE BUFFERS
        // - STAGING
        // - GPU ONLY
        //----------------------
        m_bufferType = vk::BufferUsageFlagBits::eVertexBuffer;
        Utils::Logger::LogInfoVerboseOnly("Allocating Vertex buffer and staging buffer for the mesh...");
        CreateBuffer(vertices.size() * sizeof(ApplicationCore::Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        CreateStagingBuffer(vertices.size() * sizeof(ApplicationCore::Vertex));
        Utils::Logger::LogSuccess("Vertex Buffer and staging buffer allocated successfully");

        //----------------------------
        // MAP DATA TO STAGING BUFFER
        //----------------------------
        Utils::Logger::LogInfoVerboseOnly("Filling buffer with vertex data...");
        vmaMapMemory(m_device.GetAllocator(), m_stagingAllocation, &m_mappedData);
        memcpy(m_mappedData, vertices.data(), vertices.size() * sizeof(ApplicationCore::Vertex));
        vmaUnmapMemory(m_device.GetAllocator() , m_stagingAllocation);
        Utils::Logger::LogSuccess("Vertex Buffer filled successfully...");

        //-----------------------------------------------------
        // COPY THE DATA FROM STAGING BUFFER TO GPU ONLY BUFFER
        //-----------------------------------------------------
        Utils::Logger::LogInfoVerboseOnly("Transferring data from staging buffer to the buffer located on the GPU...");
        VulkanUtils::CopyBuffers(m_device, m_stagingBufferVMA, m_bufferVMA, vertices.capacity()*sizeof(ApplicationCore::Vertex));
        Utils::Logger::LogSuccess("Transferring to GPU buffer completed...");

        //-------------------------------------------------------
        // FINISHING UP
        //-------------------------------------------------------
        m_bufferVK = vk::Buffer(m_bufferVMA);
        m_isInitialized = true;
        Utils::Logger::LogSuccess("Vertex buffer created !");
        DestroyStagingBuffer();

    }

    void VBuffer::MakeIndexBuffer(const std::vector<uint32_t> &indices) {
        assert(!m_isInitialized);

        //---------------------
        // CREATE BUFFERS
        // - STAGING
        // - GPU ONLY
        //----------------------
        m_bufferType = vk::BufferUsageFlagBits::eIndexBuffer;
        Utils::Logger::LogInfo("Allocating Index buffer and staging buffer for the mesh ");
        CreateBuffer(indices.size() * sizeof(ApplicationCore::Vertex), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        CreateStagingBuffer(indices.size() * sizeof(ApplicationCore::Vertex));
        Utils::Logger::LogInfoVerboseOnly("Index Buffer allocated successfully");

        //----------------------------
        // MAP DATA TO STAGING BUFFER
        //----------------------------
        Utils::Logger::LogInfoVerboseOnly("Filling Index buffer with vertex data");
        vmaMapMemory(m_device.GetAllocator(), m_stagingAllocation, &m_mappedData);
        memcpy(m_mappedData, indices.data(), indices.size() * sizeof(ApplicationCore::Vertex));
        vmaUnmapMemory(m_device.GetAllocator() , m_stagingAllocation);
        Utils::Logger::LogInfoVerboseOnly("Index Buffer mapped successfully");

        //-----------------------------------------------------
        // COPY THE DATA FROM STAGING BUFFER TO GPU ONLY BUFFER
        //-----------------------------------------------------
        Utils::Logger::LogInfoVerboseOnly("Transferring data from staging buffer to the buffer located on the GPU !");
        VulkanUtils::CopyBuffers(m_device, m_stagingBufferVMA, m_bufferVMA, indices.capacity()*sizeof(ApplicationCore::Vertex));
        Utils::Logger::LogInfoVerboseOnly("Transferring to GPU buffer completed");

        //-------------------------------------------------------
        // FINISHING UP
        //-------------------------------------------------------
        m_bufferVK = vk::Buffer(m_bufferVMA);
        m_isInitialized = true;
        Utils::Logger::LogSuccess("Index buffer created successfully ");
        DestroyStagingBuffer();
    }

    void VBuffer::Destroy() {
        if(m_isPresistentlyMapped) {
            vmaUnmapMemory(m_device.GetAllocator(), m_allocation);
        }

        vmaDestroyBuffer(m_device.GetAllocator(), m_bufferVMA, m_allocation);
    }

    void VBuffer::CreateStagingBuffer(VkDeviceSize size) {

        std::string allocationNme = "Allocation of staging buffer for " + m_allocationName;

        VkBufferCreateInfo stagingBufferCreateInfo = {};
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.size = size;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        stagingBufferCreateInfo.queueFamilyIndexCount = m_sharedQueueFamilyIndices.size();
        stagingBufferCreateInfo.pQueueFamilyIndices = m_sharedQueueFamilyIndices.data();

        VmaAllocationCreateInfo stagingAllocationCreateInfo = {};
        stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocationCreateInfo.flags =  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT ;
        stagingAllocationCreateInfo.priority = 1.0f;


        Utils::Logger::LogInfoVerboseOnly("Creating staging buffer...");
        assert(vmaCreateBuffer(m_device.GetAllocator(),&stagingBufferCreateInfo, &stagingAllocationCreateInfo, &m_stagingBufferVMA, &m_stagingAllocation,nullptr) == VK_SUCCESS);
        m_stagingBufferVK = m_stagingBufferVMA;

        vmaSetAllocationName(m_device.GetAllocator(), m_stagingAllocation, allocationNme.c_str());

        Utils::Logger::LogSuccess("Staging buffer created");
    }

    void VBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
        VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.queueFamilyIndexCount = m_sharedQueueFamilyIndices.size();
        bufferCreateInfo.pQueueFamilyIndices = m_sharedQueueFamilyIndices.data();


        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        assert(vmaCreateBuffer(m_device.GetAllocator(),&bufferCreateInfo,&allocationCreateInfo, &m_bufferVMA,&m_allocation,nullptr) == VK_SUCCESS);

        vmaSetAllocationName(m_device.GetAllocator(), m_allocation, m_allocationName.c_str());
        Utils::Logger::LogSuccess("Buffer allocated successfully");
    }

    void VBuffer::DestroyStagingBuffer() const {
        vmaDestroyBuffer(m_device.GetAllocator(), m_stagingBufferVMA, m_stagingAllocation);
    }

} // VulkanCore