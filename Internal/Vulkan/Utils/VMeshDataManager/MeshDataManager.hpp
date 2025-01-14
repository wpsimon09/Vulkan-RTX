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

    class MeshDatatManager : public VObject
    {
    public:
        explicit MeshDatatManager(const VulkanCore::VDevice& device);

        VulkanStructs::MeshData AddMeshData(std::vector<ApplicationCore::Vertex>& vertices, std::vector<uint32_t>& indices);

        /**
         * Sends every staging buffer to the GPU in a batch
         * @param semaphore semaphore to secure that transfering the vertex buffer is done
         */
        void UpdateGPU(vk::Semaphore semaphore);

        void Destroy() override;

        ~MeshDatatManager() = default;
    private:
        std::vector<ApplicationCore::Vertex> m_stagingVertices;
        std::vector<uint32_t> m_stagingIndices;

        //==================================
        // VERTEX BUFFER
        //==================================
        VulkanStructs::GPUBufferInfo* m_currentVertexBuffer;
        std::vector<VulkanStructs::GPUBufferInfo> m_vertexBuffers;

        //==================================
        // INDEX BUFFER
        //==================================
        VulkanStructs::GPUBufferInfo* m_currentIndexBuffer;
        std::vector<VulkanStructs::GPUBufferInfo> m_indexBuffers;

        //==================================
        // BOUNDING BOX VERTEX BUFFER
        //==================================
        VulkanStructs::GPUBufferInfo* m_currentVertexBuffer_BB;
        std::vector<VulkanStructs::GPUBufferInfo> m_vertexBuffers_BB; // to visualize bounding box

        //=========================================
        // BOUNDING BOX INDEX - is allways the same
        //========================================
        VulkanStructs::GPUBufferInfo m_indexBuffer_BB;  // to visualize bounding box

        std::unique_ptr<VulkanCore::VCommandPool> m_transferCommandPool;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCommandBuffer;

        const VulkanCore::VDevice& m_device;

    private:

        //=========================================================================================
        // FUNCTIONS THAT GENERATES STRUCTS THAT DEFINE OFFSET AND SIZE OF THE VERTEX SUB - BUFFER
        //=========================================================================================
        VulkanStructs::BufferInfo GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::BufferInfo GenerateVertexBuffer_BB(VulkanStructs::Bounds& bounds);
        VulkanStructs::BufferInfo GenerateIndexBuffer(const std::vector<uint32_t>& indices);

        //==============================================
        // FUNCTIONS THAT WILL ALLOCATE 16mb BIG BUFFER
        //==============================================
        void CreateNewVertexBuffers();
        void CreateNewIndexBuffers();
        // this is special case of the functions below that just specify how to allocated staging buffer
        VulkanStructs::StagingBufferInfo CreateStagingBuffer(VkDeviceSize size) const;

        // HELPER FUNCTION THAT IS CALLED FROM CreateNewIndexBuffer
        void CreateBuffer(VulkanStructs::GPUBufferInfo& allocationInfo) const;

        // HELPER FUNCTION THAT DELETES STAGING BUFFER
        void DeleteAllStagingBuffers();

        VulkanStructs::Bounds CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices);
    };

} // VulkanCore

#endif //VBUFFERALLOCATOR_HPP
