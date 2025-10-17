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
#include <memory>

#include "Vulkan/Global/EngineOptions.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VGrowableBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
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
{

    Utils::Logger::LogInfoVerboseOnly("Allocating VertexBuffer");

    m_indexBufferHandle = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_4_MB);

    m_indexBufferHandle->Allocate(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                                  | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc
                                  | vk::BufferUsageFlagBits::eShaderDeviceAddress
                                  | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
    m_vertexBufferHandle = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_32_MB);

    m_vertexBufferHandle->Allocate(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
                                   | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress
                                   | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

    m_readBackCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, m_device.GetTransferCommandPool());


    //m_indexBufferHandle_BB = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_16_MB);

    //m_vertexBufferHandl_BB = std::make_unique<VGrowableBuffer>(device, VulkanCore::SIZE_16_MB);
}

VulkanStructs::VMeshData2 MeshDatatManager::AddMeshData(std::vector<ApplicationCore::Vertex>& vertices, std::vector<uint32_t>& indices)
{

    auto                      bounds   = ApplicationCore::CalculateBounds(vertices);
    VulkanStructs::VMeshData2 meshData = {};

    meshData.vertexData = GenerateVertexBuffer(vertices);
    meshData.indexData  = GenerateIndexBuffer(indices);
    meshData.bounds     = bounds;
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
    bufferInfo.index                            = m_vertexSubAllocations.size();

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

    m_stagingIndices.insert(m_stagingIndices.end(), indices.begin(), indices.end());

    VulkanStructs::VGPUSubBufferInfo bufferInfo = {.size          = indices.size() * sizeof(uint32_t),
                                                   .offset        = m_indexBuffer.currentOffset,
                                                   .buffer        = m_indexBufferHandle->GetHandle().buffer,
                                                   .bufferAddress = m_indexBufferHandle->GetHandle().bufferAddress};
    bufferInfo.index                            = m_indexSubAllocations.size();

    m_indexBuffer.currentOffset += indices.size() * sizeof(uint32_t);

    m_indexSubAllocations.push_back(bufferInfo);
    return &m_indexSubAllocations.back();
}
void MeshDatatManager::OnIndexBufferResized(VulkanStructs::BufferHandle& newHandle)
{
    for(auto& subAlloc : m_indexSubAllocations)
    {
        subAlloc.buffer = newHandle.buffer;
    }
}
void MeshDatatManager::OnVertexBufferResized(VulkanStructs::BufferHandle& newHandle)
{
    for(auto& subAlloc : m_vertexSubAllocations)
    {
        subAlloc.buffer = newHandle.buffer;
    }
}
void MeshDatatManager::OnVertexBufferDeleted(vk::DeviceSize removedRegionSize, VulkanStructs::VGPUSubBufferInfo* subBuffer)
{

    // since on std::list [i] does not work i have to manually advance iterator
    auto it = m_vertexSubAllocations.begin();
    std::advance(it, subBuffer->index + 1);

    for(; it != m_vertexSubAllocations.end(); ++it)
    {
        auto& subAlloc = *it;
        subAlloc.offset -= removedRegionSize;
        subAlloc.index--;
    }

    auto itDel = m_vertexSubAllocations.begin();
    std::advance(itDel, subBuffer->index);
    m_vertexSubAllocations.erase(itDel);
}
void MeshDatatManager::OnIndexBufferDeleted(vk::DeviceSize removedRegionSize, VulkanStructs::VGPUSubBufferInfo* subBuffer)
{
    auto it = m_indexSubAllocations.begin();
    std::advance(it, subBuffer->index + 1);

    for(; it != m_indexSubAllocations.end(); ++it)
    {
        auto& subAlloc = *it;
        subAlloc.offset -= removedRegionSize;
        subAlloc.index--;
    }

    auto itDel = m_indexSubAllocations.begin();
    std::advance(itDel, subBuffer->index);
    m_indexSubAllocations.erase(itDel);
}

void MeshDatatManager::UpdateGPU()
{
    auto& cmdBuffer = m_device.GetTransferOpsManager().GetCommandBuffer();

    assert(cmdBuffer.GetIsRecording()
           && "Command buffer is not recording any commands, before using it make sure it is in recording state  !");

    //============================================
    // VERTEX STAGING BUFFER
    //============================================
    if(m_stagingVertices.size() > 0)
    {
        m_vertexBufferHandle->PushBack(m_stagingVertices.data(), m_stagingVertices.size() * sizeof(ApplicationCore::Vertex),
                                       std::bind(&MeshDatatManager::OnVertexBufferResized, this, std::placeholders::_1));

        //place barrier so that everything that reads this have to wait until it is done
        VulkanUtils::VBarrierPosition barrierPos = {
            vk::PipelineStageFlagBits2::eCopy,
            vk::AccessFlagBits2::eTransferWrite,
            vk::PipelineStageFlagBits2::eVertexAttributeInput | vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            vk::AccessFlagBits2::eVertexAttributeRead | vk::AccessFlagBits2::eAccelerationStructureReadKHR,
        };

        VulkanUtils::PlaceBufferMemoryBarrier2(cmdBuffer.GetCommandBuffer(), m_vertexBufferHandle->GetHandle().buffer, barrierPos);
    }
    //============================================
    // VERTEX_BB STAGING BUFFER
    //============================================


    //============================================
    // INDEX STAGING BUFFER
    //============================================
    if(m_stagingIndices.size() > 0)
    {

        m_indexBufferHandle->PushBack(m_stagingIndices.data(), m_stagingIndices.size() * sizeof(uint32_t),
                                      std::bind(&MeshDatatManager::OnIndexBufferResized, this, std::placeholders::_1));

        VulkanUtils::VBarrierPosition barrierPos = {
            vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
            vk::PipelineStageFlagBits2::eIndexInput | vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            vk::AccessFlagBits2::eIndexRead | vk::AccessFlagBits2::eAccelerationStructureReadKHR};
        VulkanUtils::PlaceBufferMemoryBarrier2(cmdBuffer.GetCommandBuffer(), m_indexBufferHandle->GetHandle().buffer, barrierPos);
        //=========================================================================================================================================
    }

    // CLEAN UP ONCE ALL DATA ARE IN GPU
    {

        m_stagingIndices.clear();
        m_stagingVertices.clear();
    }
}


void MeshDatatManager::Destroy()
{
    m_vertexBufferHandle->Destroy();
    m_indexBufferHandle->Destroy();
}

void MeshDatatManager::ProcessRemove(VulkanStructs::VMeshData2& meshData)
{
    m_vertexBufferHandle->Remove(meshData.vertexData->offset, meshData.vertexData->size,
                                 std::bind(&MeshDatatManager::OnVertexBufferDeleted, this, std::placeholders::_1,
                                           meshData.vertexData));
    m_indexBufferHandle->Remove(meshData.indexData->offset, meshData.indexData->size,
                                std::bind(&MeshDatatManager::OnIndexBufferDeleted, this, std::placeholders::_1, meshData.indexData));
}


VulkanStructs::VReadBackBufferInfo<ApplicationCore::Vertex> MeshDatatManager::ReadBackVertexBuffer()
{

    auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

    VulkanStructs::VReadBackBufferInfo<ApplicationCore::Vertex> vertexReadBackBufferInfos;

    // create staging buffer to copy readback memory from
    vertexReadBackBufferInfos.size = m_vertexBufferHandle->GetCurrentOffset();
    vertexReadBackBufferInfos.data.resize(m_vertexBufferHandle->GetCurrentOffset() / sizeof(ApplicationCore::Vertex));

    auto stagingBuffer          = VulkanUtils::CreateStagingBuffer(m_device, m_vertexBufferHandle->GetCurrentOffset());
    stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;

    VulkanUtils::CopyBuffers(m_device, *bufferCopiedFence, m_vertexBufferHandle->GetHandle().buffer,
                             stagingBuffer.m_stagingBufferVK, m_vertexBufferHandle->GetCurrentOffset(), 0, 0);
    bufferCopiedFence->WaitForFence();
    bufferCopiedFence->ResetFence();
    memcpy(vertexReadBackBufferInfos.data.data(), stagingBuffer.mappedPointer, m_vertexBuffer.currentOffset);

    vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
    vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);

    bufferCopiedFence->Destroy();
    return vertexReadBackBufferInfos;
}

VulkanStructs::VReadBackBufferInfo<uint32_t> MeshDatatManager::ReadBackIndexBuffers()
{

    auto bufferCopiedFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device);

    VulkanStructs::VReadBackBufferInfo<uint32_t> indexReadBackBuffer;

    // create staging buffer to copy readback memory from

    indexReadBackBuffer.size = m_indexBufferHandle->GetCurrentOffset();
    indexReadBackBuffer.data.resize(m_indexBufferHandle->GetCurrentOffset() / sizeof(uint32_t));

    auto stagingBuffer          = VulkanUtils::CreateStagingBuffer(m_device, m_indexBufferHandle->GetCurrentOffset());
    stagingBuffer.copyDstBuffer = stagingBuffer.m_stagingBufferVK;

    VulkanUtils::CopyBuffers(m_device, *bufferCopiedFence, m_indexBufferHandle->GetHandle().buffer,
                             stagingBuffer.m_stagingBufferVK, m_indexBufferHandle->GetCurrentOffset(), 0, 0);
    bufferCopiedFence->WaitForFence();
    bufferCopiedFence->ResetFence();
    memcpy(indexReadBackBuffer.data.data(), stagingBuffer.mappedPointer, m_indexBuffer.currentOffset);

    vmaUnmapMemory(m_device.GetAllocator(), stagingBuffer.m_stagingAllocation);
    vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer.m_stagingBufferVMA, stagingBuffer.m_stagingAllocation);

    bufferCopiedFence->Destroy();
    return indexReadBackBuffer;
}
}  // namespace VulkanCore