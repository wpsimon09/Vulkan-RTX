//
// Created by wpsimon09 on 02/05/25.
//

#include "VShaderStorageBuffer.hpp"

namespace VulkanCore {
VShaderStorageBuffer::VShaderStorageBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize)
    : m_device(device)
    , m_bufferSize(initialSize)
    , m_currentSize(0)
{
}
void VShaderStorageBuffer::Allocate()
{
    std::string allocationNme = "Allocation of staging buffer for Shader storage buffer";

    //============================================================================================
    // CREATE HOST VISIBLE BUFFER THAT CAN BE USED TO LOAD DATA INTO
    //============================================================================================
    VkBufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.usage              = static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eTransferSrc);
    stagingBufferCreateInfo.size               = static_cast<VkDeviceSize>(m_bufferSize);
    stagingBufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    std::vector<uint32_t> shaderQueueIndices   = {
        m_device.GetQueueFamilyIndices().transferFamily.value().second,
        m_device.GetQueueFamilyIndices().graphicsFamily.value().second,
        m_device.GetQueueFamilyIndices().computeFamily.value().second,
    };
    stagingBufferCreateInfo.queueFamilyIndexCount = shaderQueueIndices.size();
    stagingBufferCreateInfo.pQueueFamilyIndices   = shaderQueueIndices.data();

    VmaAllocationCreateInfo stagingAllocationCreateInfo = {};
    stagingAllocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    stagingAllocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    stagingAllocationCreateInfo.priority                = 1.0f;


    Utils::Logger::LogInfoVerboseOnly("Creating staging buffer...");

    VulkanUtils::Check(static_cast<vk::Result>(vmaCreateBuffer(m_device.GetAllocator(), &stagingBufferCreateInfo, &stagingAllocationCreateInfo,
                                                               &m_stagingBuffer.buffer, &m_stagingBuffer.allocation, nullptr)));

    vmaSetAllocationName(m_device.GetAllocator(), m_stagingBuffer.allocation, allocationNme.c_str());


    //==========================
    // MAP HOST VISIBLE POINTER
    //==========================
    vmaMapMemory(m_device.GetAllocator(), m_stagingBuffer.allocation, &m_stagingBuffer.mappedPtr);

    //============================================================
    // ALLOCATE DEVICE ONLY BUFFER
    //============================================================
    VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size               = m_bufferSize;
    bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                             | vk::BufferUsageFlagBits::eTransferDst);
    ;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    bufferCreateInfo.queueFamilyIndexCount = shaderQueueIndices.size();
    bufferCreateInfo.pQueueFamilyIndices   = shaderQueueIndices.data();


    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    VulkanUtils::Check(static_cast<vk::Result>(vmaCreateBuffer(m_device.GetAllocator(), &bufferCreateInfo, &allocationCreateInfo,
                                                               &m_buffer.buffer, &m_buffer.allocation, nullptr)));


    allocationNme = "Device only buffer for the data";
    vmaSetAllocationName(m_device.GetAllocator(), m_buffer.allocation, allocationNme.c_str());
    Utils::Logger::LogSuccess("Buffer allocated successfully || SIZE: " + std::to_string(m_bufferSize) + " bytes || ");

    vk::BufferDeviceAddressInfo bufferAdressInfo;
    bufferAdressInfo.buffer = m_buffer.buffer;
    m_deviceAddress         = m_device.GetDevice().getBufferAddress(bufferAdressInfo);
}
 vk::Buffer VShaderStorageBuffer::GetBuffer() const {
    return m_buffer.buffer;
}

void VShaderStorageBuffer::Resize(vk::DeviceSize newSize)
{
    assert(0 == 1 && "Not implemented yet !");
}
void VShaderStorageBuffer::Destroy()
{
    vmaUnmapMemory(m_device.GetAllocator(), m_stagingBuffer.allocation);
    vmaDestroyBuffer(m_device.GetAllocator(), m_stagingBuffer.buffer, m_stagingBuffer.allocation);
    vmaDestroyBuffer(m_device.GetAllocator(), m_buffer.buffer, m_buffer.allocation);
}
vk::DeviceSize VShaderStorageBuffer::GetCurrentSize() const {return m_currentSize; }


} // VulkanCore