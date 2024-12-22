//
// Created by wpsimon09 on 20/10/24.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.h>

#include "Application/Logger/Logger.hpp"
#include "VMA/vk_mem_alloc.h"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace ApplicationCore
{
    struct Vertex;
}

namespace VulkanCore
{
    class VGraphicsPipeline;

    class VBuffer : public VObject
    {
    public:
        explicit VBuffer(const VDevice &device,const std::string& name = "");

        const vk::Buffer &GetBuffer() const { return m_bufferVK; }
        const vk::Buffer &GetStagingBuffer() const { return m_stagingBufferVK; }

        void *GetMapPointer() const {
            assert(m_bufferType == vk::BufferUsageFlagBits::eUniformBuffer);
            return m_mappedData;
        }

        void *MapStagingBuffer();

        void UnMapStagingBuffer();

        vk::DescriptorBufferInfo &GetBufferInfoForDescriptor();

        void MakeVertexBuffer(const std::vector<ApplicationCore::Vertex> &vertices);

        void MakeIndexBuffer(const std::vector<uint32_t> &indices);

        template <typename T>
        void MakeUniformBuffer(const T &uniformBuffer, vk::DeviceSize size);

        void Destroy() override;

        void CreateStagingBuffer(VkDeviceSize size);

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

        void DestroyStagingBuffer() const;

        ~VBuffer() override = default;

    private:
        const VDevice &m_device;

        // normal buffer
        VmaAllocation m_allocation;
        vk::Buffer m_bufferVK;
        VkBuffer m_bufferVMA;

        // staging buffer
        VmaAllocation m_stagingAllocation;
        VkBuffer m_stagingBufferVMA;
        vk::Buffer m_stagingBufferVK;

        // others
        vk::BufferUsageFlags m_bufferType;
        std::vector<uint32_t> m_sharedQueueFamilyIndices;
        vk::DescriptorBufferInfo m_descriptorBufferInfo;
        const std::string m_allocationName;

        bool m_isInitialized = false;
        bool m_isPresistentlyMapped = false;
        void *m_mappedData;

    };


    template <typename T>
    void VBuffer::MakeUniformBuffer(const T &uniformBuffer, vk::DeviceSize size) {

        m_isPresistentlyMapped = true;
        assert(!m_isInitialized);

        //---------------------
        // CREATE BUFFERS
        // - GPU<->CPU VISIBLE
        // - persistently mapped
        //----------------------
        Utils::Logger::LogInfoVerboseOnly("Allocating Uniform buffer....");
        m_bufferType = vk::BufferUsageFlagBits::eUniformBuffer;
        CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
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
        m_bufferVK = vk::Buffer(m_bufferVMA);
        m_isInitialized = true;
        Utils::Logger::LogSuccess("Uniform buffer created !");

        m_descriptorBufferInfo.buffer = m_bufferVK;
        m_descriptorBufferInfo.range = size;
        m_descriptorBufferInfo.offset = 0;

    }


} // VulkanCore

#endif //VVERTEXBUFFER_HPP
