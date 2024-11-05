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

namespace VulkanCore {
    class VGraphicsPipeline;

    class VBuffer:public VObject {
    public:
        explicit VBuffer(const VDevice& device);

        const vk::Buffer& GetBuffer() const {return m_bufferVK; }

        void* GetMapPointer() const { assert(m_bufferType == vk::BufferUsageFlagBits::eUniformBuffer); return m_mappedData;}

        vk::DescriptorBufferInfo &GetBufferInfoForDescriptor();

        void MakeVertexBuffer(const std::vector< ApplicationCore::Vertex>& vertices);

        void MakeIndexBuffer(const std::vector< uint32_t>& indices);

        template<typename T>
        void MakeUniformBuffer(const T& uniformBuffer, vk::DeviceSize size);

        void MakeImageBuffer();

        void Destroy() override;
        ~VBuffer() override = default;
    private:
        void CreateStagingBuffer(VkDeviceSize size);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
        void DestroyStagingBuffer() const;
    private:
        const VDevice& m_device;
        bool m_isInitialized = false;
        bool m_isPresistentlyMapped = false;
        void* m_mappedData;
        VmaAllocation m_allocation;
        VkBuffer m_bufferVMA;
        VmaAllocation m_stagingAllocation;
        VkBuffer m_stagingBuffer;
        vk::Buffer m_bufferVK;
        vk::BufferUsageFlags m_bufferType;
        std::vector<uint32_t> m_sharedQueueFamilyIndices;
        vk::DescriptorBufferInfo m_descriptorBufferInfo;;
    };

    template <typename T>
    void VBuffer::MakeUniformBuffer(const T& uniformBuffer, vk::DeviceSize size) {
        m_isPresistentlyMapped = true;
        assert(!m_isInitialized);

        //---------------------
        // CREATE BUFFERS
        // - GPU<->CPU VISIBLE
        // - persistently mapped
        //----------------------
        Utils::Logger::LogInfoVerboseOnly("Allocating Uniform buffer....");
        m_bufferType = vk::BufferUsageFlagBits::eUniformBuffer;
        CreateBuffer(size,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
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
        DestroyStagingBuffer();

        m_descriptorBufferInfo.buffer = m_bufferVK;
        m_descriptorBufferInfo.range = size;
        m_descriptorBufferInfo.offset = 0;

    }


} // VulkanCore

#endif //VVERTEXBUFFER_HPP
