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

#include <list>


namespace VulkanCore {
class VGrowableBuffer;
}
namespace VulkanUtils {
class VTransferOperationsManager;
}

namespace VulkanCore {
class VCommandBuffer;
}

namespace VulkanCore {
class VCommandPool;
}

namespace ApplicationCore {
struct Vertex;
}

namespace VulkanCore {
class VDevice;

class MeshDatatManager : public VObject
{
  public:
    explicit MeshDatatManager(const VulkanCore::VDevice& device);

    VulkanStructs::VMeshData2 AddMeshData(std::vector<ApplicationCore::Vertex>& vertices, std::vector<uint32_t>& indices);

    /**
         * Sends every staging buffer to the GPU in a batch
         * @param semaphore semaphore to secure that transfering the vertex buffer is done
         */
    void UpdateGPU(vk::Semaphore semaphore);

    void Destroy() override;

    std::vector<ApplicationCore::Vertex>                      ReadBack(VulkanStructs::VGPUSubBufferInfo& bufferInfo);
    std::vector<VulkanStructs::VReadBackBufferInfo<uint32_t>> ReadBackIndexBuffers();
    std::vector<VulkanStructs::VReadBackBufferInfo<ApplicationCore::Vertex>> ReadBackVertexBuffer();

    ~MeshDatatManager() = default;

  private:
    // i am coping this whole thing to the one buffer which is not good if new buffer was allocated
    // use map to know which staging vertices go to which buffer
    // TODO: might be even better to use vkBuffer as an index to the map
    std::vector<ApplicationCore::Vertex> m_stagingVertices;
    std::vector<ApplicationCore::Vertex> m_stagingVertices_BB;
    std::vector<uint32_t>                m_stagingIndices;

    //==================================
    // VERTEX BUFFER
    //==================================
    VulkanStructs::VGPUBufferInfo m_vertexBuffer;
    std::unique_ptr<VulkanCore::VGrowableBuffer> m_vertexBufferHandle;
    std::list<VulkanStructs::VGPUSubBufferInfo> m_vertexSubAllocations;

    //==================================
    // INDEX BUFFER
    //==================================
    VulkanStructs::VGPUBufferInfo m_indexBuffer;
    std::unique_ptr<VulkanCore::VGrowableBuffer> m_indexBufferHandle;
    std::list<VulkanStructs::VGPUSubBufferInfo> m_indexSubAllocations;


    //==================================
    // BOUNDING BOX VERTEX BUFFER
    //==================================
    std::unique_ptr<VulkanCore::VGrowableBuffer> m_vertexBufferHandl_BB;
    VulkanStructs::VGPUBufferInfo m_vertexBuffers_BB;  // to visualize bounding box
    std::vector<VulkanStructs::VGPUSubBufferInfo> m_vertexSubAllocations_BB;

    //=========================================
    // BOUNDING BOX INDEX - is allways the same
    //========================================
    std::unique_ptr<VulkanCore::VGrowableBuffer> m_indexBufferHandle_BB;
    VulkanStructs::VGPUBufferInfo m_indexBuffer_BB;  // to visualize bounding box
    std::vector<VulkanStructs::VGPUSubBufferInfo> m_indexSubAllocations_BB;

    //===========================================
    // Transfer pool
    //===========================================
    std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCommandBuffer;
    const VulkanCore::VDevice&                  m_device;

  private:
    //=========================================================================================
    // FUNCTIONS THAT GENERATES STRUCTS THAT DEFINE OFFSET AND SIZE OF THE SUB - BUFFER
    //=========================================================================================
    VulkanStructs::VGPUSubBufferInfo* GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices);
    VulkanStructs::VGPUSubBufferInfo* GenerateVertexBuffer_BB(VulkanStructs::VBounds& bounds);

    VulkanStructs::VGPUSubBufferInfo*        GenerateIndexBuffer(const std::vector<uint32_t>& indices);
    VulkanUtils::VTransferOperationsManager& m_transferOpsManager;

    //==============================================
    // FUNCTIONS THAT WILL ALLOCATE 16mb BIG BUFFER
    //==============================================
};

}  // namespace VulkanCore

#endif  //VBUFFERALLOCATOR_HPP
