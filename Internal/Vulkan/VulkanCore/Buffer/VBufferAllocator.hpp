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

        VulkanStructs::BufferInfo AddVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::BufferInfo AddIndexBuffer(const std::vector<uint32_t>& indices);

        /**
         * Sends every staging buffer to the GPU in a batch
         * @param semaphore semaphore to secure that transfering the vertex buffer is done
         * TODO:this might be wrong since graphis and transfer queue have the same queue family and i should put the memory barrier since it might be moved to be single queue
         */
        void UpdateGPU(vk::Semaphore semaphore);

        void Destroy() override;

        ~VBufferAllocator() = default;
    private:
        //==================================
        // VERTEX BUFFER
        //==================================
        VulkanStructs::BufferAllocationInfo* m_currentVertexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_vertexBuffers;
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_stagingVertexBuffers;

        //==================================
        // INDEX BUFFER
        //==================================
        VulkanStructs::BufferAllocationInfo* m_currentIndexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_indexBuffers;
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_stagingIndexBuffers;

        //==================================
        // BOUNDING BOX VERTEX BUFFER
        //==================================
        VulkanStructs::BufferAllocationInfo* m_currentBBvertexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_BBvertexBuffers; // to visualize bounding box
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_BBstagingVertexBuffers;

        //==================================
        // BOUNDING BOX INDEX
        //==================================
        VulkanStructs::BufferAllocationInfo* m_currentBBindexBuffer;
        std::vector<VulkanStructs::BufferAllocationInfo> m_BBindexBuffers;  // to visualize bounding box
        std::vector<VulkanStructs::StagingBufferAllocationInfo> m_BBstagingIndexBuffers;

        std::unique_ptr<VulkanCore::VCommandPool> m_transferCommandPool;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCommandBuffer;

        const VulkanCore::VDevice& m_device;

        void CreateNewVertexBuffers();
        void CreateNewIndexBuffers();
        void CreateBuffer(VulkanStructs::BufferAllocationInfo& allocationInfo);
        void ClearVertexStagingBuffers();
        VulkanStructs::Bounds CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::StagingBufferAllocationInfo CreateStagingBuffer(VkDeviceSize size);
    };
} // VulkanCore

#endif //VBUFFERALLOCATOR_HPP
