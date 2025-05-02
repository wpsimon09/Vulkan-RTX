//
// Created by wpsimon09 on 02/05/25.
//

#ifndef VSHADERSTORAGEBUFFER_HPP
#define VSHADERSTORAGEBUFFER_HPP
#include "Application/Logger/Logger.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"


#include <vulkan/vulkan.hpp>

namespace VulkanCore {

class VDevice;

class VShaderStorageBuffer : public VObject
{
  public:
    VShaderStorageBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize);

    void Allocate(vk::BufferUsageFlags usage);

    /**
     * Sends data to the GPU and checks if the handle is still capable of fitting in withint the buffer,
     * if not buffer will be resized to accomodate for this
     */
    void Update();

    void* GetUpdateHandle();

    /**
     * Resizes the buffer and puts new data to the GPU
     * @param newSize new size of the buffer
     */
    void Resize(vk::DeviceSize newSize);


  private:
    struct BufferHandle
    {
        VmaAllocation allocation{};
        VkBuffer      buffer{};
        void*         mappedPtr;
    } m_stagingBuffer, m_buffer;

  private:
    const VulkanCore::VDevice& m_device;

    vk::DeviceSize m_bufferSize;
    vk::DeviceSize m_currentSize;
    vk::DeviceAddress m_deviceAddress;
};

}  // namespace VulkanCore

#endif  //VSHADERSTORAGEBUFFER_HPP
