//
// Created by wpsimon09 on 07/01/25.
//

#ifndef VBUFFERALLOCATOR_HPP
#define VBUFFERALLOCATOR_HPP

#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"


namespace VulkanCore
{
    class VCommandBuffer;
}

namespace VulkanCore
{
    class VCommandPool;
}

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
        VulkanStructs::BufferInfo AddIndexBuffer(std::vector<uint32_t>& indices);

        void UpdateGPU(vk::Semaphore semaphore);

        void Destroy() override;

        ~VBufferAllocator() = default;
    private:
        VulkanStructs::BufferAllocationInfo* m_currentVertexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_vertexBuffers;
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_stagingVertexBuffers;

        VulkanStructs::BufferAllocationInfo* m_currentIndexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_indexBuffers;
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_stagingIndexBuffers;

        VulkanStructs::BufferAllocationInfo* m_currentBBvertexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_BBvertexBuffers; // to visualize bounding box
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_BBstagingVertexBuffers;

        VulkanStructs::BufferAllocationInfo* m_currentBBindexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_BBindexBuffers;  // to visualize bounding box
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_BBstagingIndexBuffers;

        std::unique_ptr<VulkanCore::VCommandPool> m_transferCommandPool;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCommandBuffer;

        const VulkanCore::VDevice& m_device;

        void CreateNewVertexBuffers();
        void CreateNewIndexBuffers();
        void CreateBuffer(VulkanStructs::BufferAllocationInfo& allocationInfo);
        VulkanStructs::StagingBufferAllocationInfo CreateStagingBuffer(VkDeviceSize size);
    };
} // VulkanCore

#endif //VBUFFERALLOCATOR_HPP
