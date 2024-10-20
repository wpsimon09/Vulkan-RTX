//
// Created by wpsimon09 on 20/10/24.
//

#include "VBuffer.hpp"

#include "Application/Rendering/Mesh/Mesh.hpp"

namespace VulkanCore {

    VBuffer::VBuffer(const VDevice &device): VObject(), m_device(device) {
    }

    const void VBuffer::MakeVertexBuffer(const ApplicationCore::Mesh &mesh) const {
        vk::BufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.size = mesh.GetMeshVertexArraySize();
        bufferCreateInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;


    }

    void VBuffer::Destroy() {
        VObject::Destroy();
    }


} // VulkanCore