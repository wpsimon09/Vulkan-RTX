//
// Created by wpsimon09 on 13/01/25.
//
#include "VFactories.hpp"

#include "Vulkan/VulkanCore/Buffer/VBufferAllocator.hpp"

namespace VulkanUtils
{
    VulkanStructs::BufferInfo CreateVertexBuffer(VulkanCore::VBufferAllocator& bufferAllocator,
        const std::vector<ApplicationCore::Vertex>& vertices)
    {
        return bufferAllocator.AddVertexBuffer(vertices);
    }

    VulkanStructs::BufferInfo CreateIndexBuffer(VulkanCore::VBufferAllocator& bufferAllocator,
        std::vector<uint32_t>& indices)
    {
        return bufferAllocator.AddIndexBuffer(indices);
    }
}
