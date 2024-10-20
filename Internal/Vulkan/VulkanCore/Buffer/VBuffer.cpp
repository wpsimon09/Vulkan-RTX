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

    void VBuffer::MakeVertexBuffer(const ApplicationCore::Mesh &mesh) {
        Utils::Logger::LogInfo("Allocating Vertex buffer for the mesh");

        VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = mesh.GetMeshVertexArraySize();
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        assert(vmaCreateBuffer(m_device.GetAllocator(),&bufferCreateInfo,&allocationCreateInfo, &m_bufferVMA,&m_allocation,nullptr) == VK_SUCCESS);
        Utils::Logger::LogSuccess("Vertex Buffer allocated successfully");

        Utils::Logger::LogInfoVerboseOnly("Filling buffer with vertex data");
        vmaMapMemory(m_device.GetAllocator(), m_allocation, &m_mappedData);
        memcpy(m_mappedData, mesh.GetVertexArray().GetVertices().data(), mesh.GetVertexArray().GetVertices().size());
        assert(sizeof(m_mappedData) > 0);
        vmaUnmapMemory(m_device.GetAllocator() , m_allocation);

        m_bufferVK = vk::Buffer(m_bufferVMA);
        Utils::Logger::LogSuccess("Vertex buffer filled successfully ");
    }

    void VBuffer::Destroy() {
        vmaDestroyBuffer(m_device.GetAllocator(), m_bufferVMA, m_allocation);
        m_device.GetDevice().destroyBuffer(m_bufferVK);
    }


} // VulkanCore