//
// Created by wpsimon09 on 20/10/24.
//

#ifndef VVERTEXBUFFER_HPP
#define VVERTEXBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>
#include "VMA/vk_mem_alloc.h"

namespace ApplicationCore
{
    class Mesh;
}

namespace VulkanCore {
    class VDevice;
    class VGraphicsPipeline;

    class VBuffer:public VObject {
    public:
        VBuffer(const VDevice& device);

        const void MakeVertexBuffer(const ApplicationCore::Mesh& mesh) const;
        void MakeIndexBuffer();
        void MakeImageBuffer();
        void MakeUniformBuffer();

        void Destroy() override;
        ~VBuffer() override = default;
    private:
        const VDevice& m_device;
        bool m_isInitialized = false;
        VmaAllocation m_allocation = nullptr;
        vk::Buffer m_buffer;
    };
} // VulkanCore

#endif //VVERTEXBUFFER_HPP
