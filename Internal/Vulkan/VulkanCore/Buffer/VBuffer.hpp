//
// Created by wpsimon09 on 20/10/24.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.h>

#include "Application/Logger/Logger.hpp"
#include "VMA/vk_mem_alloc.h"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"

namespace ApplicationCore
{
    struct Vertex;
}

namespace VulkanCore {
    class VDevice;
    class VGraphicsPipeline;

    class VBuffer:public VObject {
    public:
        explicit VBuffer(const VDevice& device);

        const vk::Buffer& GetBuffer() const {return m_bufferVK; }
        const void* GetMapPointer() const { assert(m_bufferType == vk::BufferUsageFlagBits::eUniformBuffer); return m_mappedData;}

        void MakeVertexBuffer(const std::vector< ApplicationCore::Vertex>& vertices);
        void MakeIndexBuffer(const std::vector< uint32_t>& indices);

        template<typename T>
        void MakeUniformBuffer(T uniformBuffer);
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
        void* m_mappedData;
        VmaAllocation m_allocation;
        VkBuffer m_bufferVMA;
        VmaAllocation m_stagingAllocation;
        VkBuffer m_stagingBuffer;
        vk::Buffer m_bufferVK;
        vk::BufferUsageFlags m_bufferType;
        std::vector<uint32_t> m_sharedQueueFamilyIndices;
    };

    template <typename T>
    void VBuffer::MakeUniformBuffer(T uniformBuffer) {
        VkDeviceSize size = sizeof(uniformBuffer);
        assert(!m_isInitialized);

        //---------------------
        // CREATE BUFFERS
        // - GPU<->CPU VISIBLE
        //----------------------
        Utils::Logger::LogInfoVerboseOnly("Allocating Vertex buffer and staging buffer for the mesh...");
        m_bufferType = vk::BufferUsageFlagBits::eUniformBuffer;
        CreateBuffer(size,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        Utils::Logger::LogSuccess("Vertex Buffer and staging buffer allocated successfully");

        //----------------------------
        // MAP DATA TO STAGING BUFFER
        //----------------------------
        Utils::Logger::LogInfoVerboseOnly("Mapping uniform buffer...");
        vmaMapMemory(m_device.GetAllocator(), m_allocation, &m_mappedData);
        Utils::Logger::LogSuccess("Unifomr buffer mapped successfully");

        //-------------------------------------------------------
        // FINISHING UP
        //-------------------------------------------------------
        m_bufferVK = vk::Buffer(m_bufferVMA);
        m_isInitialized = true;
        Utils::Logger::LogSuccess("Uniform buffer created !");
        DestroyStagingBuffer();

    }


} // VulkanCore

#endif //VVERTEXBUFFER_HPP
