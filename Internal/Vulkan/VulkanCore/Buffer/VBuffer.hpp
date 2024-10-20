//
// Created by wpsimon09 on 20/10/24.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.h>
#include "VMA/vk_mem_alloc.h"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"

namespace ApplicationCore
{
    class Mesh;
}

namespace VulkanCore {
    class VDevice;
    class VGraphicsPipeline;

    class VBuffer:public VObject {
    public:
        explicit VBuffer(const VDevice& device);

        const void MakeVertexBuffer(const ApplicationCore::Mesh& mesh) const;
        void MakeIndexBuffer();
        void MakeImageBuffer();
        void MakeUniformBuffer();

        void GetBuffer();

        void Destroy() override;
        ~VBuffer() override = default;
    private:
        const VDevice& m_device;
        bool m_isInitialized = false;
        void** m_mappedData;
        VmaAllocation* m_allocation = nullptr;
        VkBuffer* m_buffer;
    };
} // VulkanCore

#endif //VVERTEXBUFFER_HPP
