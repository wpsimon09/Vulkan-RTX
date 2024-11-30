//
// Created by wpsimon09 on 04/10/24.
//

#include "VertexArray.hpp"

#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


ApplicationCore::VertexArray::VertexArray(const VulkanCore::VDevice &device, PRIMITIVE_TOPOLOGY topology,
    const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices):m_vertices(vertices), m_indices(indices) {
    m_topology = topology;

    m_vertexBuffer = std::make_unique<VulkanCore::VBuffer>(device, "<== VERTEX BUFFER ==>");
    m_indexBuffer = std::make_unique<VulkanCore::VBuffer>(device, "<== INDEX BUFFER ==>");

    m_indexBuffer->MakeIndexBuffer(m_indices);
    m_vertexBuffer->MakeVertexBuffer(m_vertices);
}

void ApplicationCore::VertexArray::Destroy() const {
    m_vertexBuffer->Destroy();
    m_indexBuffer->Destroy();
}
