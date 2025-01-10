//
// Created by wpsimon09 on 07/01/25.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include <VMA/vk_mem_alloc.h>

#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"

#ifndef BUFFER_SIZE
    #define BUFFER_SIZE 16777216 // 16MB
#endif

namespace ApplicationCore
{
    struct Vertex;
}

namespace VulkanCore
{
    class VDevice;

    class VBufferAllocator : public VObject
    {
    public:
        VBufferAllocator(const VulkanCore::VDevice& device);


        VulkanStructs::BufferInfo& AddVertexBuffer(std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::BufferInfo& AddIndexBuffer(std::vector<ApplicationCore::Vertex>& vertices);
    private:
        struct BufferAllocationInfo
        {
            vk::DeviceSize size;
            vk::DeviceSize currentOffset;
            vk::Buffer bufferVK;
            vk::BufferUsageFlags usageFlags;
            VkBuffer bufferVMA;
            VmaAllocation allocationVMA;

            vk::DeviceSize GetAvailableSize() {return size - currentOffset;};
        };

        std::vector<BufferAllocationInfo> m_vertexBuffers;
        std::vector<BufferAllocationInfo> m_indexBuffers;
        std::vector<BufferAllocationInfo> m_BBvertexBuffers; // to visualize bounding box
        std::vector<BufferAllocationInfo> m_BBindexBuffers; // to visualize bounding box

        VmaAllocation m_stagingAllocation;
        VkBuffer m_stagingBufferVMA;
        vk::Buffer m_stagingBufferVK;

        const VulkanCore::VDevice& m_device;

        void CreateBuffer(BufferAllocationInfo& allocationInfo);
    };
} // VulkanCore

#endif //VVERTEXBUFFER_HPP
