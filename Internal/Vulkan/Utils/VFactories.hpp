//
// Created by wpsimon09 on 13/01/25.
//

#ifndef VFACTORIES_HPP
#define VFACTORIES_HPP
#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore
{
    struct Vertex;
}

namespace VulkanCore
{
    class VBufferAllocator;
}
namespace VulkanUtils
{
    VulkanStructs::BufferInfo CreateVertexBuffer(VulkanCore::VBufferAllocator& bufferAllocator,
                                                 const std::vector<ApplicationCore::Vertex>& vertices);

    VulkanStructs::BufferInfo CreateIndexBuffer(VulkanCore::VBufferAllocator& bufferAllocator,
                                                std::vector<uint32_t>& indices);
}


#endif //VFACTORIES_HPP
