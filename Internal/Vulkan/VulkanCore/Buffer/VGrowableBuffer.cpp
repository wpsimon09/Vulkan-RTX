//
// Created by wpsimon09 on 23/08/2025.
//

#include "VGrowableBuffer.hpp"
#include "vulkan/vulkan.h"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VGrowableBuffer::VGrowableBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize, vk::DeviceSize chunkSize)
    : m_device(device)
{
    m_bufferSize    = initialSize;
    m_chunkSize     = chunkSize;
    m_currentOffset = 0;
    m_availabelSize = initialSize;

    m_sharedQueueIndices = {
        m_device.GetQueueFamilyIndices().transferFamily.value().second,
        m_device.GetQueueFamilyIndices().graphicsFamily.value().second,
        m_device.GetQueueFamilyIndices().computeFamily.value().second,
    };

}

void VGrowableBuffer::Allocate(vk::BufferUsageFlags usage)
{
    std::string allocationNme = "Allocation of staging buffer for Shader storage buffer";

    //============================================================
    // ALLOCATE DEVICE ONLY BUFFER
    //============================================================
    VkBufferCreateInfo bufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size               = m_bufferSize;
    bufferCreateInfo.usage              = static_cast<VkBufferUsageFlags>(usage);

    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    bufferCreateInfo.queueFamilyIndexCount = m_sharedQueueIndices.size();
    bufferCreateInfo.pQueueFamilyIndices   = m_sharedQueueIndices.data();


    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    VulkanUtils::Check(static_cast<vk::Result>(vmaCreateBuffer(m_device.GetAllocator(), &bufferCreateInfo, &allocationCreateInfo,
                                                               &m_handle.buffer, &m_handle.allocation, nullptr)));

    allocationNme = "Device only buffer for the data";
    vmaSetAllocationName(m_device.GetAllocator(), m_handle.allocation, allocationNme.c_str());
    Utils::Logger::LogSuccess("Buffer allocated successfully || SIZE: " + std::to_string(m_bufferSize) + " bytes || ");

    vk::BufferDeviceAddressInfo bufferAdressInfo;
    bufferAdressInfo.buffer = m_handle.buffer;
    m_address         = m_device.GetDevice().getBufferAddress(bufferAdressInfo);
}
void VGrowableBuffer::UpdateSizes(vk::DeviceSize size) {
    m_availabelSize -= size;
    m_currentOffset += size;
}
void VGrowableBuffer::ClearUpStaging() {
    m_device.GetTransferOpsManager().DestroyBuffer(m_scratchBuffer.m_stagingBufferVMA, m_scratchBuffer.m_stagingAllocation);
}
}  // namespace VulkanCore