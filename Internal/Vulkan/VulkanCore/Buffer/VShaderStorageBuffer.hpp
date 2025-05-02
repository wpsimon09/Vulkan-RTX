//
// Created by wpsimon09 on 02/05/25.
//

#ifndef VSHADERSTORAGEBUFFER_HPP
#define VSHADERSTORAGEBUFFER_HPP
#include "Application/Logger/Logger.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"


#include <vulkan/vulkan.hpp>

namespace VulkanCore {

class VDevice;

class VShaderStorageBuffer : public VObject
{
  public:
    VShaderStorageBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize);

    void Allocate();

    const vk::Buffer& GetBuffer() const;

    /**
     * Sends data to the GPU and checks if the handle is still capable of fitting in withint the buffer,
     * if not buffer will be resized to accomodate for this
     */
    template <typename T>
    void Update(const std::vector<T>& data);

    /**
     * Resizes the buffer and puts new data to the GPU
     * @param newSize new size of the buffer
     */
    void Resize(vk::DeviceSize newSize);

    void Destroy() override;

    vk::DeviceSize  GetCurrentSize() const ;

  private:
    struct BufferHandle
    {
        VmaAllocation allocation{};
        VkBuffer      buffer{};
        void*         mappedPtr;
    } m_stagingBuffer, m_buffer;

  private:
    const VulkanCore::VDevice& m_device;

    vk::DeviceSize    m_bufferSize;
    vk::DeviceSize    m_currentSize;
    vk::DeviceAddress m_deviceAddress;
};

template <typename T>
void VShaderStorageBuffer::Update(const std::vector<T>& data)
{
    size_t updateSize = data.size() * sizeof(T);

    assert((m_bufferSize - m_currentSize) > updateSize && "Buffer is not possible to update, i have to implement the resize feature");

    m_currentSize = updateSize;

    memcpy(m_stagingBuffer.mappedPtr, data.data(), data.size() * sizeof(T));

    VulkanUtils::CopyBuffers(m_device.GetTransferOpsManager().GetCommandBuffer().GetCommandBuffer(),
                             m_stagingBuffer.buffer, m_buffer.buffer, updateSize, 0);

}

}  // namespace VulkanCore

#endif  //VSHADERSTORAGEBUFFER_HPP
