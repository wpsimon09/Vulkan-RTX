//
// Created by wpsimon09 on 07/01/25.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"

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
        VBufferAllocator(const VulkanCore::VDevice& device);


        VulkanStructs::BufferInfo& AddVertexBuffer(std::vector<ApplicationCore::Vertex>& vertices);
        VulkanStructs::BufferInfo& AddIndexBuffer(std::vector<ApplicationCore::Vertex>& vertices);
    private:
        std::vector<vk::Buffer> m_vertexBuffers;
        std::vector<vk::Buffer> m_indexBuffers;
        std::vector<vk::Buffer> m_AABBvertexBuffers;
        std::vector<vk::Buffer> m_AABBindexBuffers;
    };
} // VulkanCore

#endif //VVERTEXBUFFER_HPP
