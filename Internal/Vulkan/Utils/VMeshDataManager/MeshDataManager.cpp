//
// Created by wpsimon09 on 07/01/25.
//

#include "MeshDataManager.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Application/Utils/ApplicationUtils.hpp"
#include <limits>

#include "Vulkan/Global/EngineOptions.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VGrowableBuffer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"


#define SIZE_64MB 67108864
#define SIZE_500MB 524288000

namespace VulkanCore {
MeshDatatManager::MeshDatatManager(const VulkanCore::VDevice& device)
    : m_vertexBuffer{}
    , m_indexBuffer{}
    , m_vertexBuffers_BB{}
    , m_indexBuffer_BB{}
    , m_device(device)
    , m_transferOpsManager(device.GetTransferOpsManager())

{

    Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");

    m_indexBufferHandle    = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_16_MB);

    m_indexBufferHandle->Allocate(vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR  | vk::BufferUsageFlagBits::eTransferDst
                                 | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress
                                    | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR );
    m_vertexBufferHandle   = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_16_MB);

    m_vertexBufferHandle->Allocate(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
                                 | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress
                                    | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR );
    //m_indexBufferHandle_BB = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_16_MB);
    //m_vertexBufferHandl_BB = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_16_MB);
}

VulkanStructs::VMeshData2 MeshDatatManager::AddMeshData(std::vector<ApplicationCore::Vertex>& vertices, std::vector<uint32_t>& indices)
{

    auto                      bounds   = ApplicationCore::CalculateBounds(vertices);
    VulkanStructs::VMeshData2 meshData = {};

    meshData.vertexData = GenerateVertexBuffer(vertices);
    meshData.indexData  = GenerateIndexBuffer(indices);
    meshData.bounds              = bounds;
    //meshData.vertexData_BB = GenerateVertexBuffer_BB(bounds);

    //meshData.indexData_BB.buffer = m_indexBuffer_BB.bufferVK;
    //meshData.indexData_BB.size   = m_indexBuffer_BB.size;
    //meshData.indexData_BB.offset = 0;

    return meshData;
}

VulkanStructs::VGPUSubBufferInfo* MeshDatatManager::GenerateVertexBuffer(const std::vector<ApplicationCore::Vertex>& vertices)
{
    //=======================================================================================
    // RETURNS A STRUCT THAT SPECIFIES WHERE THE VERTICES ARE STORED IN 16mb CHUNK OF MEMORY
    // IF VERTICES WONT FIT IT WILL CREATE NEW BUFFER AND PUTS THE VERTICES THERE
    //=======================================================================================
    m_stagingVertices.insert(m_stagingVertices.end(), vertices.begin(), vertices.end());

    VulkanStructs::VGPUSubBufferInfo bufferInfo = {.size          = vertices.size() * sizeof(ApplicationCore::Vertex),
                                                   .offset        = m_vertexBuffer.currentOffset,
                                                   .buffer        = m_vertexBufferHandle->GetHandle().buffer,
                                                   .bufferAddress = m_vertexBufferHandle->GetHandle().bufferAddress};
    bufferInfo.index = m_vertexSubAllocations.size();

    m_vertexBuffer.currentOffset += vertices.size() * sizeof(ApplicationCore::Vertex);
    m_vertexSubAllocations.push_back(std::move(bufferInfo));

    return &m_vertexSubAllocations.back();
}

VulkanStructs::VGPUSubBufferInfo* MeshDatatManager::GenerateVertexBuffer_BB(VulkanStructs::VBounds& bounds)
{
    /*
    std::vector<ApplicationCore::Vertex> Vertices_BB = {
        {bounds.origin + glm::vec3(-bounds.extents.x, -bounds.extents.y, -bounds.extents.z), {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // V0
        {bounds.origin + glm::vec3(+bounds.extents.x, -bounds.extents.y, -bounds.extents.z), {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},  // V1
        {bounds.origin + glm::vec3(+bounds.extents.x, +bounds.extents.y, -bounds.extents.z), {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},  // V2
        {bounds.origin + glm::vec3(-bounds.extents.x, +bounds.extents.y, -bounds.extents.z), {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},  // V3
        {bounds.origin + glm::vec3(-bounds.extents.x, -bounds.extents.y, +bounds.extents.z), {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},  // V4
        {bounds.origin + glm::vec3(+bounds.extents.x, -bounds.extents.y, +bounds.extents.z), {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // V5
        {bounds.origin + glm::vec3(+bounds.extents.x, +bounds.extents.y, +bounds.extents.z), {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},  // V6
        {bounds.origin + glm::vec3(-bounds.extents.x, +bounds.extents.y, +bounds.extents.z), {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // V7
    };

    m_stagingVertices_BB[0].insert(m_stagingVertices_BB[0].end(), Vertices_BB.begin(), Vertices_BB.end());

    VulkanStructs::VGPUSubBufferInfo bufferInfo = {.size   = Vertices_BB.size() * sizeof(ApplicationCore::Vertex),
                                                  .offset = m_currentVertexBuffer_BB->currentOffset,
                                                  .buffer = m_currentVertexBuffer_BB->bufferVK,
                                                  .ID = VulkanUtils::random_int(1, std::numeric_limits<int>::max() - 1),
                                                  .BufferID      = m_currentVertexBuffer_BB->ID,
                                                  .bufferAddress = m_currentVertexBuffer_BB->bufferAddress};

    m_currentVertexBuffer_BB->currentOffset += Vertices_BB.size() * sizeof(ApplicationCore::Vertex);

    return bufferInfo;
    */
    return nullptr;
}


VulkanStructs::VGPUSubBufferInfo* MeshDatatManager::GenerateIndexBuffer(const std::vector<uint32_t>& indices)
{

    m_stagingIndices.insert(m_stagingIndices.begin(), indices.begin(), indices.end());

    VulkanStructs::VGPUSubBufferInfo bufferInfo = {.size          = indices.size() * sizeof(uint32_t),
                                                   .offset        =  m_indexBuffer.currentOffset,
                                                   .buffer        =m_indexBufferHandle->GetHandle().buffer,
                                                   .bufferAddress =  m_indexBufferHandle->GetHandle().bufferAddress};
    bufferInfo.index = m_indexSubAllocations.size();

    m_indexBuffer.currentOffset += indices.size() * sizeof(uint32_t);

    m_indexSubAllocations.push_back(bufferInfo);
    assert(bufferInfo.buffer != nullptr && "Garbage data detected !");
    return &m_indexSubAllocations.back();
}

void MeshDatatManager::UpdateGPU(vk::Semaphore semaphore)
{

    assert(m_transferOpsManager.GetCommandBuffer().GetIsRecording()
           && "Command buffer is not recording any commands, before using it make sure it is in recording state  !");
    auto& cmdBuffer = m_transferOpsManager.GetCommandBuffer().GetCommandBuffer();

    Utils::Logger::LogSuccess("Buffer copy of vertex and index buffer completed !");
    //=========================================================================================================================================
    // VERTEX STAGING BUFFER
    //==========================================================================================================================================
    m_vertexBufferHandle->PushBack(m_stagingVertices.data(), m_stagingVertices.size() * sizeof(ApplicationCore::Vertex));
    //=========================================================================================================================================
    // VERTEX_BB STAGING BUFFER
    //==========================================================================================================================================
    m_indexBufferHandle->PushBack(m_stagingIndices.data(), m_stagingIndices.size() * sizeof(uint32_t));

    //=========================================================================================================================================
    // INDEX STAGING BUFFER
    //==========================================================================================================================================
    // CLEAN UP ONCE ALL DATA ARE IN GPU
    {

        m_stagingIndices.clear();
        m_stagingVertices.clear();
    }
}


void MeshDatatManager::Destroy() {

}


std::vector<ApplicationCore::Vertex> MeshDatatManager::ReadBack(VulkanStructs::VGPUSubBufferInfo& bufferInfo)
{
    auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);
    std::vector<ApplicationCore::Vertex> vertices;
    vertices.resize(bufferInfo.size / sizeof(ApplicationCore::Vertex));

    // create staging buffer to copy readback memory from
    auto stagingBuffer          = VulkanUtils::CreateStagingBuffer(m_device, bufferInfo.size);
    stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;

    VulkanUtils::CopyBuffers(m_device, *bufferCopiedFence, bufferInfo.buffer, stagingBuffer.m_stagingBufferVK,
                             bufferInfo.size, bufferInfo.offset, 0);

    bufferCopiedFence->WaitForFence();
    memcpy(vertices.data(), stagingBuffer.mappedPointer, bufferInfo.size);

    vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
    vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
    bufferCopiedFence->Destroy();

    return vertices;
}

std::vector<VulkanStructs::VReadBackBufferInfo<ApplicationCore::Vertex>> MeshDatatManager::ReadBackVertexBuffer()
{

    /*
     *
    auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

    std::vector<VulkanStructs::VReadBackBufferInfo<ApplicationCore::Vertex>> vertexReadBackBufferInfos;
    vertexReadBackBufferInfos.resize(m_vertexBuffer.size());

    // create staging buffer to copy readback memory from
    for(int i = 0; i < m_vertexBuffer.size(); i++)
    {

        vertexReadBackBufferInfos[i].bufferID = m_vertexBuffer[i].ID;
        vertexReadBackBufferInfos[i].size     = m_vertexBuffer[i].size;
        vertexReadBackBufferInfos[i].data.resize(m_vertexBuffer[i].currentOffset / sizeof(ApplicationCore::Vertex));

        auto stagingBuffer          = VulkanUtils::CreateStagingBuffer(m_device, m_vertexBuffer[i].size);
        stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;

        VulkanUtils::CopyBuffers(m_device, *bufferCopiedFence, m_vertexBuffer[i].bufferVK,
                                 stagingBuffer.m_stagingBufferVK, m_vertexBuffer[i].currentOffset, 0, 0);
        bufferCopiedFence->WaitForFence();
        bufferCopiedFence->ResetFence();
        memcpy(vertexReadBackBufferInfos[i].data.data(), stagingBuffer.mappedPointer, m_vertexBuffer[i].currentOffset);

        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
        vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
    }
    bufferCopiedFence->Destroy();
    return vertexReadBackBufferInfos;
    */
}

std::vector<VulkanStructs::VReadBackBufferInfo<uint32_t>> MeshDatatManager::ReadBackIndexBuffers()
{
    /*

    auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

    std::vector<VulkanStructs::VReadBackBufferInfo<uint32_t>> indexReadBackBufferInfos;
    indexReadBackBufferInfos.resize(m_indexBuffer.size());

    // create staging buffer to copy readback memory from
    for(int i = 0; i < m_indexBuffer.size(); i++)
    {

        indexReadBackBufferInfos[i].bufferID = m_indexBuffer[i].ID;
        indexReadBackBufferInfos[i].size     = m_indexBuffer[i].size;
        indexReadBackBufferInfos[i].data.resize(m_indexBuffer[i].currentOffset / sizeof(uint32_t));

        auto stagingBuffer          = VulkanUtils::CreateStagingBuffer(m_device, m_indexBuffer[i].size);
        stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;

        VulkanUtils::CopyBuffers(m_device, *bufferCopiedFence, m_indexBuffer[i].bufferVK,
                                 stagingBuffer.m_stagingBufferVK, m_indexBuffer[i].currentOffset, 0, 0);
        bufferCopiedFence->WaitForFence();
        bufferCopiedFence->ResetFence();
        memcpy(indexReadBackBufferInfos[i].data.data(), stagingBuffer.mappedPointer, m_indexBuffer[i].currentOffset);

        vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
        vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);
    }
    bufferCopiedFence->Destroy();
    return indexReadBackBufferInfos;
    */
}
}  // namespace VulkanCore