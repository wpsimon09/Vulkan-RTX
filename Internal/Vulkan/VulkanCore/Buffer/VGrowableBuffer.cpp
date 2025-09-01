//
// Created by wpsimon09 on 23/08/2025.
//

#include "VGrowableBuffer.hpp"
#include "vulkan/vulkan.h"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VGrowableBuffer::VGrowableBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize, vk::DeviceSize chunkSize)
    : m_device(device),m_transferCmdBuffer(device.GetTransferOpsManager().GetCommandBuffer()), VObject()
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

void VGrowableBuffer::Remove(vk::DeviceSize offset, vk::DeviceSize size, OnBufferDelete onBufferDelete) {
    // where region that we want to remove ends
    vk::DeviceSize tailOffset = offset + size;

    // size of the region that is after we want to remove it
    // | AAAA | BBBB | (remove) | DDDD | EEEE (DDDD, EEEE is tail size)
    assert(tailOffset < m_bufferSize && "Tail offset is bigger then the size of the buffer which is never supposed to be a case and indicates bug somewhere else ");
    vk::DeviceSize tailSize =   m_bufferSize - tailOffset;

    m_scratchBuffer = VulkanUtils::CreateStagingBuffer(m_device, tailSize);

    VulkanUtils::CopyBuffers(m_transferCmdBuffer.GetCommandBuffer(), m_handle.buffer, m_scratchBuffer.m_stagingBufferVK, tailSize, tailOffset);

    VulkanUtils::VBarrierPosition barrierPos = {
        vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
        vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferRead,
    };
    VulkanUtils::PlaceBufferMemoryBarrier2(m_transferCmdBuffer.GetCommandBuffer(), m_scratchBuffer.m_stagingBufferVK,barrierPos);

    VulkanUtils::CopyBuffers(m_transferCmdBuffer.GetCommandBuffer(), m_scratchBuffer.m_stagingBufferVK, m_handle.buffer, tailSize, 0, offset );

    barrierPos = {
        vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
        vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR, vk::AccessFlagBits2::eTransferRead,
    };
    VulkanUtils::PlaceBufferMemoryBarrier2(m_transferCmdBuffer.GetCommandBuffer(), m_handle.buffer,barrierPos);

    ClearUpStaging();

    if (onBufferDelete) {
        // offset at which the buffer was changed
        onBufferDelete(size);
    }
}

void VGrowableBuffer::Destroy()
{
    vmaDestroyBuffer(m_device.GetAllocator(), m_handle.buffer, m_handle.allocation);
}

VulkanStructs::BufferHandle& VGrowableBuffer::GetHandle() {
    return m_handle;
}

void VGrowableBuffer::Resize(vk::DeviceSize chunkSize, const OnBufferResize& onBufferResize) {
    //==========================================================
    // create new buffer that will be used as a copy destination
    Utils::Logger::LogInfo("Resiting buffer...");
    auto newBuffer = VulkanUtils::CreateBuffer(m_device, m_bufferUsage, m_bufferSize + chunkSize);

    VulkanUtils::CopyBuffers(m_transferCmdBuffer.GetCommandBuffer(), m_handle.buffer, newBuffer.buffer, m_handle.size);

    m_device.GetTransferOpsManager().DestroyBuffer(m_handle.buffer, m_handle.allocation);

    m_bufferSize = newBuffer.size;
    m_availabelSize +=  newBuffer.size;
    m_handle = newBuffer;

    // callback
    if (onBufferResize) {
        onBufferResize(m_handle);
    }
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