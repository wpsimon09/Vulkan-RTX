//
// Created by wpsimon09 on 07/01/25.
//

#ifndef VBUFFERALLOCATOR_HPP
#define VBUFFERALLOCATOR_HPP

#include <map>

#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"


namespace VulkanUtils
{
    class VTransferOperationsManager;
}

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

        std::vector<ApplicationCore::Vertex> ReadBack(VulkanStructs::GPUSubBufferInfo& bufferInfo);
        std::vector<VulkanStructs::ReadBackBufferInfo<uint32_t>> ReadBackIndexBuffers();
        std::vector<VulkanStructs::ReadBackBufferInfo<ApplicationCore::Vertex>> ReadBackVertexBuffer();

        ~MeshDatatManager() = default;
    private:

        // i am coping this whole thing to the one buffer which is not good if new buffer was allocated
        // use map to know which staging vertices go to which buffer
        // TODO: might be even better to use vkBuffer as an index to the map
        std::map<int , std::vector<ApplicationCore::Vertex>> m_stagingVertices;
        std::map<int , std::vector<ApplicationCore::Vertex>> m_stagingVertices_BB;
        std::map<int , std::vector<uint32_t>> m_stagingIndices;

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


        //===========================================
        // Transfer pool
        //===========================================
        std::unique_ptr<VulkanCore::VCommandPool> m_transferCommandPool;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCommandBuffer;
        const VulkanCore::VDevice& m_device;

    private:

        //=========================================================================================
        // FUNCTIONS THAT GENERATES STRUCTS THAT DEFINE OFFSET AND SIZE OF THE SUB - BUFFER
        //=========================================================================================
        VulkanStructs::GPUSubBufferInfo GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::GPUSubBufferInfo GenerateVertexBuffer_BB(VulkanStructs::Bounds& bounds);

        VulkanStructs::GPUSubBufferInfo GenerateIndexBuffer(const std::vector<uint32_t>& indices);
        VulkanUtils::VTransferOperationsManager& m_transferOpsManager;

        //==============================================
        // FUNCTIONS THAT WILL ALLOCATE 16mb BIG BUFFER
        //==============================================
        void CreateNewVertexBuffers(bool createForBoundingBox = false);
        void CreateNewIndexBuffers();
        void SelectMostSuitableBuffer(EBufferType bufferType, vk::DeviceSize subAllocationSize);

        // HELPER FUNCTION THAT IS CALLED FROM CreateNewIndexBuffer
        void CreateBuffer(VulkanStructs::GPUBufferInfo& allocationInfo) const;

    };

} // VulkanCore

#endif //VBUFFERALLOCATOR_HPP
