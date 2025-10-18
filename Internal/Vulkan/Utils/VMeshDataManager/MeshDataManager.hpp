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
         * Sends every staging buffer to the GPU in a batch records transfer command buffer with data in the staging vertices and staging indcies 
         */
    void UpdateGPU();

    void Destroy() override;

    void ProcessRemove(VulkanStructs::VMeshData2& subAllocation);

    void Reset();

    bool                                                        WasResized();
    VulkanStructs::VReadBackBufferInfo<uint32_t>                ReadBackIndexBuffers();
    VulkanStructs::VReadBackBufferInfo<ApplicationCore::Vertex> ReadBackVertexBuffer();

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
    VulkanStructs::VGPUBufferInfo                m_vertexBuffer;
    std::unique_ptr<VulkanCore::VGrowableBuffer> m_vertexBufferHandle;
    std::list<VulkanStructs::VGPUSubBufferInfo>  m_vertexSubAllocations;

    //==================================
    // INDEX BUFFER
    //==================================
    VulkanStructs::VGPUBufferInfo                m_indexBuffer;
    std::unique_ptr<VulkanCore::VGrowableBuffer> m_indexBufferHandle;
    std::list<VulkanStructs::VGPUSubBufferInfo>  m_indexSubAllocations;

    const VulkanCore::VDevice& m_device;

  private:
    //=========================================================================================
    // FUNCTIONS THAT GENERATES STRUCTS THAT DEFINE OFFSET AND SIZE OF THE SUB - BUFFER
    //=========================================================================================
    VulkanStructs::VGPUSubBufferInfo* GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices);
    VulkanStructs::VGPUSubBufferInfo* GenerateIndexBuffer(const std::vector<uint32_t>& indices);

    std::unique_ptr<VulkanCore::VCommandBuffer> m_readBackCommandBuffer;

    //=========================================
    // Callbacks
    void OnIndexBufferResized(VulkanStructs::BufferHandle& newHandle);
    void OnVertexBufferResized(VulkanStructs::BufferHandle& newHandle);

    void OnVertexBufferDeleted(vk::DeviceSize removedRegionSize, VulkanStructs::VGPUSubBufferInfo* subBuffer);
    void OnIndexBufferDeleted(vk::DeviceSize removedRegionSize, VulkanStructs::VGPUSubBufferInfo* subBuffer);

    // was any mesh buffer resized ? if yes we have to rebuild AS
    bool m_wasResized = false;
};

}  // namespace VulkanCore

#endif  //VBUFFERALLOCATOR_HPP
