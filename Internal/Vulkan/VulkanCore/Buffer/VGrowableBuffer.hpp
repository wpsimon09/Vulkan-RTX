//
// Created by wpsimon09 on 23/08/2025.
//

#ifndef VULKAN_RTX_VGROWABLEBUFFER_HPP
#define VULKAN_RTX_VGROWABLEBUFFER_HPP

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"


#include <vulkan/vulkan.hpp>
namespace VulkanCore {

static constexpr size_t SIZE_4_MB  = 4194304;
static constexpr size_t SIZE_8_MB  = 8388608;
static constexpr size_t SIZE_16_MB = 16777216;
static constexpr size_t SIZE_32_MB = 33554432;
static constexpr size_t SIZE_64_MB = 16777264;

class VDevice;

class VGrowableBuffer : public VulkanCore::VObject
{
  public:
    VGrowableBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize, vk::DeviceSize chunkSize = SIZE_8_MB);
    void Allocate(vk::BufferUsageFlags usage);

    template <typename T>
    void Fill(T* data, vk::DeviceSize size);

    template <typename T>
    void PushBack(T* data, vk::DeviceSize size);

    void Remove(vk::DeviceSize offset, vk::DeviceSize size);

    void Destroy() override;

    VulkanStructs::BufferHandle& GetHandle();

  private:
    void Resize(vk::DeviceSize chunkSize);
    void UpdateSizes(vk::DeviceSize size);
    void ClearUpStaging();

  private:
    const VulkanCore::VDevice& m_device;
    vk::DeviceSize             m_bufferSize;
    vk::DeviceSize             m_chunkSize;
    vk::DeviceSize             m_currentOffset;
    vk::DeviceSize             m_availabelSize;
    vk::BufferUsageFlags       m_bufferUsage;

    const VulkanCore::VCommandBuffer& m_transferCmdBuffer;

    // scratch buffer is used to store temporary information and for staging
    VulkanStructs::BufferHandle                   m_handle;
    VulkanStructs::VStagingBufferInfo             m_scratchBuffer;
    std::vector<VulkanStructs::VGPUSubBufferInfo> m_subAllocations;
    std::vector<uint32_t>                         m_sharedQueueIndices;

    vk::DeviceAddress m_address;
};

template <typename T>
void VGrowableBuffer::Fill(T* data, vk::DeviceSize size)
{
    // Check if this data will fit the buffer
    if(size > m_availabelSize)
    {
        Resize(m_chunkSize);
    }

    // allocate scratch buffer
    m_scratchBuffer = VulkanUtils::CreateStagingBuffer(m_device, size);

    // fill the scratch buffer
    memcpy(m_scratchBuffer.mappedPointer, data, size);

    // 0 offset since this will rewrite everything in the buffer
    VulkanUtils::CopyBuffers(m_transferCmdBuffer.GetCommandBuffer(), m_scratchBuffer.m_stagingBufferVK, m_handle.buffer, size);

    UpdateSizes(size);
    ClearUpStaging();
}

template <typename T>
void VGrowableBuffer::PushBack(T* data, vk::DeviceSize size)
{
    // Check if this data will fit the buffer
    if(size > m_availabelSize)
    {
        Resize(m_chunkSize);
    }

    // allocate scratch buffer
    m_scratchBuffer = VulkanUtils::CreateStagingBuffer(m_device, size);

    // fill the scratch buffer
    memcpy(m_scratchBuffer.mappedPointer, data, size);

    // 0 offset since this will rewrite everything in the buffer
    VulkanUtils::CopyBuffers(m_transferCmdBuffer.GetCommandBuffer(), m_scratchBuffer.m_stagingBufferVK, m_handle.buffer,
                             size, 0, m_currentOffset);

    UpdateSizes(size);
    ClearUpStaging();
}


}  // namespace VulkanCore

#endif  //VULKAN_RTX_VGROWABLEBUFFER_HPP
