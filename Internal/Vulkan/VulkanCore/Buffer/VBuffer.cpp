//
// Created by wpsimon09 on 20/10/24.
//

#include "VBuffer.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "vulkan/vulkan.h"

namespace VulkanCore {

VBuffer::VBuffer(const VDevice& device, const std::string& name)
    : VObject()
    , m_device(device)
    , m_allocationName(name)
{
    m_sharedQueueFamilyIndices = {device.GetQueueFamilyIndices().graphicsFamily.value().second,
                                  device.GetQueueFamilyIndices().transferFamily.value().second};
    m_isInitialized            = false;
    m_isPresistentlyMapped     = false;
}

void* VBuffer::MapStagingBuffer()
{
    Utils::Logger::LogInfoVerboseOnly("Mapping staging buffer...");
    vmaMapMemory(m_device.GetAllocator(), m_stagingAllocation, &m_mappedData);
    return m_mappedData;
}

void VBuffer::UnMapStagingBuffer()
{
    Utils::Logger::LogInfoVerboseOnly("Unmapping staging buffer...");
    vmaUnmapMemory(m_device.GetAllocator(), m_stagingAllocation);
}

vk::DescriptorBufferInfo& VBuffer::GetBufferInfoForDescriptor()
{
    assert(m_bufferType == vk::BufferUsageFlagBits::eUniformBuffer
           && "This buffer is not uniform buffer and therefore can not be used in descriptors");
    return m_descriptorBufferInfo;
}


void VBuffer::Destroy()
{
    if(m_isPresistentlyMapped)
    {
        vmaUnmapMemory(m_device.GetAllocator(), m_allocation);
    }

    vmaDestroyBuffer(m_device.GetAllocator(), m_bufferVMA, m_allocation);
}

void VBuffer::CreateHostVisibleBuffer(VkDeviceSize size, VkBufferUsageFlags usage, uint32_t aligment)
{

    std::string allocationNme = "Allocation of staging buffer for " + m_allocationName;

    VkBufferCreateInfo stagingBufferCreateInfo    = {};
    stagingBufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.usage                 = usage;
    stagingBufferCreateInfo.size                  = size;
    stagingBufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    stagingBufferCreateInfo.queueFamilyIndexCount = m_sharedQueueFamilyIndices.size();
    stagingBufferCreateInfo.pQueueFamilyIndices   = m_sharedQueueFamilyIndices.data();

    VmaAllocationCreateInfo stagingAllocationCreateInfo = {};
    stagingAllocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    stagingAllocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    stagingAllocationCreateInfo.priority                = 1.0f;


    Utils::Logger::LogInfoVerboseOnly("Creating staging buffer...");

    if(aligment == 0)
    {

        VulkanUtils::Check(static_cast<vk::Result>(vmaCreateBuffer(m_device.GetAllocator(), &stagingBufferCreateInfo, &stagingAllocationCreateInfo,
                                                                   &m_stagingBufferVMA, &m_stagingAllocation, nullptr)));
    }
    else
    {
        VulkanUtils::Check(static_cast<vk::Result>(
            vmaCreateBufferWithAlignment(m_device.GetAllocator(), &stagingBufferCreateInfo, &stagingAllocationCreateInfo,
                                         aligment, &m_stagingBufferVMA, &m_stagingAllocation, nullptr)));
    }
    m_stagingBufferVK = m_stagingBufferVMA;

    vmaSetAllocationName(m_device.GetAllocator(), m_stagingAllocation, allocationNme.c_str());

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        vk::BufferDeviceAddressInfo bufferAdressInfo;
        bufferAdressInfo.buffer = m_stagingBufferVK;
        m_bufferAddress         = m_device.GetDevice().getBufferAddress(bufferAdressInfo);
    }

    Utils::Logger::LogSuccess("Staging buffer created || SIZE: " + std::to_string(size) + "bytes ||");
}

void VBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferCreateInfo    = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size                  = size;
    bufferCreateInfo.usage                 = usage;
    bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = m_sharedQueueFamilyIndices.size();
    bufferCreateInfo.pQueueFamilyIndices   = m_sharedQueueFamilyIndices.data();


    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    assert(vmaCreateBuffer(m_device.GetAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_bufferVMA, &m_allocation, nullptr)
           == VK_SUCCESS);


    m_bufferVK   = m_bufferVMA;
    m_bufferSize = size;

    vmaSetAllocationName(m_device.GetAllocator(), m_allocation, m_allocationName.c_str());
    Utils::Logger::LogSuccess("Buffer allocated successfully || SIZE: " + std::to_string(size) + " bytes || ");

    vk::BufferDeviceAddressInfo bufferAdressInfo;
    bufferAdressInfo.buffer = m_bufferVK;
    m_bufferAddress         = m_device.GetDevice().getBufferAddress(bufferAdressInfo);
}
void VBuffer::CreateBufferWithAligment(VkDeviceSize size, VkBufferUsageFlags usage, vk::DeviceSize minAligment)
{
    VkBufferCreateInfo bufferCreateInfo    = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size                  = size;
    bufferCreateInfo.usage                 = usage;
    bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = m_sharedQueueFamilyIndices.size();
    bufferCreateInfo.pQueueFamilyIndices   = m_sharedQueueFamilyIndices.data();


    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;


    VulkanUtils::Check(static_cast<vk::Result>(vmaCreateBufferWithAlignment(
        m_device.GetAllocator(), &bufferCreateInfo, &allocationCreateInfo, minAligment, &m_bufferVMA, &m_allocation, nullptr)));

    m_bufferVK   = m_bufferVMA;
    m_bufferSize = size;

    vmaSetAllocationName(m_device.GetAllocator(), m_allocation, m_allocationName.c_str());
    Utils::Logger::LogSuccess("Buffer allocated successfully || SIZE: " + std::to_string(size) + " bytes || ");

    vk::BufferDeviceAddressInfo bufferAdressInfo;
    bufferAdressInfo.buffer = m_bufferVK;
    m_bufferAddress         = m_device.GetDevice().getBufferAddress(bufferAdressInfo);
}

void VBuffer::DestroyStagingBuffer() const
{
    vmaDestroyBuffer(m_device.GetAllocator(), m_stagingBufferVMA, m_stagingAllocation);
}

}  // namespace VulkanCore