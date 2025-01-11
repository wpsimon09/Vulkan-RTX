//
// Created by wpsimon09 on 07/01/25.
//

#ifndef VBUFFERALLOCATOR_HPP
#define VBUFFERALLOCATOR_HPP

#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"


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
        explicit VBufferAllocator(const VulkanCore::VDevice& device);

        VulkanStructs::BufferInfo AddVertexBuffer(std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::BufferInfo AddIndexBuffer(std::vector<ApplicationCore::Vertex>& vertices);

        void Destroy() override;

        ~VBufferAllocator() = default;
    private:

        std::vector<VulkanStructs::BufferAllocationInfo> m_vertexBuffers;
        std::vector<VulkanStructs::BufferAllocationInfo> m_indexBuffers;
        std::vector<VulkanStructs::BufferAllocationInfo> m_BBvertexBuffers; // to visualize bounding box
        std::vector<VulkanStructs::BufferAllocationInfo> m_BBindexBuffers;  // to visualize bounding box

        VmaAllocation m_stagingAllocation;
        VkBuffer m_stagingBufferVMA;
        vk::Buffer m_stagingBufferVK;

        const VulkanCore::VDevice& m_device;

        void CreateBuffer(VulkanStructs::BufferAllocationInfo& allocationInfo);
        void CreateStagingBuffer(VkDeviceSize size);
    };
} // VulkanCore

#endif //VBUFFERALLOCATOR_HPP
