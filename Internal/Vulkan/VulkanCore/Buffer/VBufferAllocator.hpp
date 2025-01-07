//
// Created by wpsimon09 on 07/01/25.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore
{
    class VDevice;

    class VBufferAllocator : public VObject
    {
    public:
        VBufferAllocator(const VulkanCore::VDevice& device);

        VulkanStructs::BufferInfo& AddBuffer();
    private:
        vk::Buffer m_vertexBuffer;
        vk::Buffer m_indexBuffer;
        vk::Buffer m_AABBvertexBuffer;
        vk::Buffer m_AABBindexBuffer;

        std::vector<VulkanStructs::BufferInfo> m_availabelBuffers;
    };
} // VulkanCore

#endif //VVERTEXBUFFER_HPP
