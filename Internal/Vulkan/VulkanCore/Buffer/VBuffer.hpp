//
// Created by wpsimon09 on 20/10/24.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.h>

#include "Application/Logger/Logger.hpp"
#include "VMA/vk_mem_alloc.h"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace ApplicationCore {
struct Vertex;
}

namespace VulkanCore {
class VGraphicsPipeline;

class VBuffer : public VObject
{
  public:
    explicit VBuffer(const VulkanCore::VDevice& device, const std::string& name = "Unspecified");

    const vk::Buffer&    GetBuffer() const { return m_bufferVK; }
    const vk::Buffer&    GetStagingBuffer() const { return m_stagingBufferVK; }
    const VmaAllocation& GetStagingBufferAllocation() const { return m_stagingAllocation; }
    vk::DeviceSize       GetBuffeSizeInBytes() const { return m_bufferSize; };
    vk::DeviceAddress    GetBufferAdress() const {return m_bufferAddress.value_or(0);}

    void* GetMapPointer() const
    {
        //assert(m_bufferType == vk::BufferUsageFlagBits::eUniformBuffer);
        return m_mappedData;
    }

    void* MapStagingBuffer();

    void UnMapStagingBuffer();

    vk::DescriptorBufferInfo& GetBufferInfoForDescriptor();

    template <typename T>
    void MakeUniformBuffer(const T& uniformBuffer, vk::DeviceSize size,  bool makeDeviceAddress = false);

    void Destroy() override;

    void CreateHostVisibleBuffer(VkDeviceSize size, VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT , uint32_t aligment = 0);

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

    void CreateBufferWithAligment(VkDeviceSize size, VkBufferUsageFlags usage, vk::DeviceSize minAligment = 128);

    template <typename T>
    void CreateBufferAndPutDataOnDevice(const vk::CommandBuffer& commandBuffer, const std::vector<T>& data, vk::BufferUsageFlags usage);

    template <typename T>
    void UpdateContents(const vk::CommandBuffer& commandBuffer, const std::vector<T>& data, vk::BufferUsageFlags usage);


    void DestroyStagingBuffer() const;

    ~VBuffer() override = default;

  private:
    const VulkanCore::VDevice& m_device;

    // normal buffer
    VmaAllocation m_allocation;
    vk::Buffer    m_bufferVK;
    VkBuffer      m_bufferVMA;

    // staging buffer
    VmaAllocation m_stagingAllocation;
    VkBuffer      m_stagingBufferVMA;
    vk::Buffer    m_stagingBufferVK;

    // others
    vk::BufferUsageFlags     m_bufferType;
    std::vector<uint32_t>    m_sharedQueueFamilyIndices;
    vk::DescriptorBufferInfo m_descriptorBufferInfo;

    vk::DeviceSize    m_bufferSize;
    const std::string m_allocationName;
    std::optional<vk::DeviceAddress> m_bufferAddress;

    bool  m_isInitialized        = false;
    bool  m_isPresistentlyMapped = false;
    bool  m_hasShaderDeviceAddress = false;
    void* m_mappedData;
};


template <typename T>
void VBuffer::MakeUniformBuffer(const T& uniformBuffer, vk::DeviceSize size, bool makeDeviceAddress)
{

    m_isPresistentlyMapped = true;
    assert(!m_isInitialized);

    //---------------------
    // CREATE BUFFERS
    // - GPU<->CPU VISIBLE
    // - persistently mapped
    //----------------------
    Utils::Logger::LogInfoVerboseOnly("Allocating Uniform buffer....");
    m_bufferType = vk::BufferUsageFlagBits::eUniformBuffer ;
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT  ;
    if (makeDeviceAddress){
        usageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ;
    }
    CreateBuffer(size, usageFlags );
    Utils::Logger::LogSuccess("Allocation completed successfully !");

    //----------------------------
    // MAP DATA TO STAGING BUFFER
    //----------------------------
    Utils::Logger::LogInfoVerboseOnly("Mapping uniform buffer...");
    vmaMapMemory(m_device.GetAllocator(), m_allocation, &m_mappedData);
    Utils::Logger::LogSuccess("Uniform buffer is going to bre persistently mapped ");

    //-------------------------------------------------------
    // FINISHING UP
    //-------------------------------------------------------
    m_bufferVK      = vk::Buffer(m_bufferVMA);
    m_isInitialized = true;
    Utils::Logger::LogSuccess("Uniform buffer created !");

    //--------------------------------------------------------
    // Get device address
    //--------------------------------------------------------

    m_descriptorBufferInfo.buffer = m_bufferVK;
    m_descriptorBufferInfo.range  = size;
    m_descriptorBufferInfo.offset = 0;
}
template <typename T>
void VBuffer::CreateBufferAndPutDataOnDevice(const vk::CommandBuffer& commandBuffer, const std::vector<T>& data, vk::BufferUsageFlags usage)
{
    CreateHostVisibleBuffer(data.size() * sizeof(T)); // implicitly maps the buffer
    CreateBuffer(data.size() * sizeof(T), static_cast<VkBufferUsageFlags>(usage) | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    memcpy(MapStagingBuffer(), data.data(), data.size() * sizeof(T));
    UnMapStagingBuffer();
    VulkanUtils::CopyBuffers(commandBuffer, m_stagingBufferVMA, m_bufferVMA, data.size() * sizeof(T));
}

}

#endif  //VVERTEXBUFFER_HPP
