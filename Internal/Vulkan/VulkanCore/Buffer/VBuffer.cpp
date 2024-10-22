//
// Created by wpsimon09 on 20/10/24.
//

#include "VBuffer.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "vulkan/vulkan.h"

namespace VulkanCore {

    VBuffer::VBuffer(const VDevice &device): VObject(), m_device(device) {
    }

    void VBuffer::MakeVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices) {
        assert(!m_isInitialized);
        Utils::Logger::LogInfo("Allocating Vertex buffer for the mesh");

        VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = vertices.size() * sizeof(ApplicationCore::Vertex);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        assert(vmaCreateBuffer(m_device.GetAllocator(),&bufferCreateInfo,&allocationCreateInfo, &m_bufferVMA,&m_allocation,nullptr) == VK_SUCCESS);
        Utils::Logger::LogSuccess("Vertex Buffer allocated successfully");

        Utils::Logger::LogInfoVerboseOnly("Filling buffer with vertex data");
        vmaMapMemory(m_device.GetAllocator(), m_allocation, &m_mappedData);
        memcpy(m_mappedData, vertices.data(), vertices.size());
        assert(sizeof(m_mappedData) > 0);
        vmaUnmapMemory(m_device.GetAllocator() , m_allocation);

        m_bufferVK = vk::Buffer(m_bufferVMA);
        m_isInitialized = true;
        Utils::Logger::LogSuccess("Vertex buffer filled successfully ");
    }

    void VBuffer::MakeIndexBuffer(const std::vector<uint32_t> &indices) {
        assert(!m_isInitialized);
        Utils::Logger::LogInfo("Allocating Vertex buffer for the mesh");

        VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = indices.size() * sizeof(uint32_t);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        assert(vmaCreateBuffer(m_device.GetAllocator(),&bufferCreateInfo,&allocationCreateInfo, &m_bufferVMA,&m_allocation,nullptr) == VK_SUCCESS);
        Utils::Logger::LogSuccess("Vertex Buffer allocated successfully");

        Utils::Logger::LogInfoVerboseOnly("Filling buffer with vertex data");
        vmaMapMemory(m_device.GetAllocator(), m_allocation, &m_mappedData);
        memcpy(m_mappedData, indices.data(), indices.size());
        assert(sizeof(m_mappedData) > 0);
        vmaUnmapMemory(m_device.GetAllocator() , m_allocation);

        m_bufferVK = vk::Buffer(m_bufferVMA);
        m_isInitialized = true;
        Utils::Logger::LogSuccess("Vertex buffer filled successfully ");
    }


    void VBuffer::Destroy() {
        vmaDestroyBuffer(m_device.GetAllocator(), m_bufferVMA, m_allocation);
        //m_device.GetDevice().destroyBuffer(m_bufferVK);
    }


} // VulkanCore