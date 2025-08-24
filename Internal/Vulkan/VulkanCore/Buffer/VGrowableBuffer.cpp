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
    : m_device(device), VObject()
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
    m_bufferUsage = usage;
    std::string allocationNme = "Allocation of staging buffer for Shader storage buffer";

    //============================================================
    // ALLOCATE DEVICE ONLY BUFFER
    //============================================================
    m_handle = VulkanUtils::CreateBuffer(m_device, usage, m_bufferSize);

}

void VGrowableBuffer::Destroy()
{
    VObject::Destroy();
}
void VGrowableBuffer::Resize(vk::DeviceSize chunkSize) {
    //==========================================================
    // create new buffer that will be used as a copy destination
    auto newBuffer = VulkanUtils::CreateBuffer(m_device, m_bufferUsage, m_bufferSize + chunkSize);

    VulkanUtils::CopyBuffers(m_device.GetTransferOpsManager().GetCommandBuffer().GetCommandBuffer(), m_handle.buffer, newBuffer.buffer, m_handle.size);

    m_device.GetTransferOpsManager().DestroyBuffer(m_handle.buffer, m_handle.allocation);

    m_bufferSize = newBuffer.size;
    m_availabelSize +=  newBuffer.size;
    m_handle = std::move(newBuffer);
}

void VGrowableBuffer::UpdateSizes(vk::DeviceSize size) {
    m_availabelSize -= size;
    m_currentOffset += size;
}
void VGrowableBuffer::ClearUpStaging() {
    vmaUnmapMemory(m_device.GetAllocator(), m_scratchBuffer.m_stagingAllocation);
    m_device.GetTransferOpsManager().DestroyBuffer(m_scratchBuffer.m_stagingBufferVMA, m_scratchBuffer.m_stagingAllocation);
}
}  // namespace VulkanCore