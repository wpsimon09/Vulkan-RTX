//
// Created by wpsimon09 on 20/10/24.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.h>
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

        const vk::Buffer& GetBuffer() const { return m_bufferVK; }

        void MakeVertexBuffer(const std::vector< ApplicationCore::Vertex>& vertices);
        void MakeIndexBuffer(const std::vector< uint32_t>& indices);
        void MakeImageBuffer();
        void MakeUniformBuffer();

        void Destroy() override;
        ~VBuffer() override = default;
    private:
        void CreateStagingBuffer(VkDeviceSize size);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlagBits usage);
    private:
        const VDevice& m_device;
        bool m_isInitialized = false;
        void* m_mappedData;
        VmaAllocation m_allocation;
        VkBuffer m_bufferVMA;
        VmaAllocation m_stagingAllocation;
        VkBuffer m_stagingBuffer;
        vk::Buffer m_bufferVK;
    };
} // VulkanCore

#endif //VVERTEXBUFFER_HPP
