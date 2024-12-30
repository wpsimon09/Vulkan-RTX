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

    m_AABBVertexBuffer = std::make_unique<VulkanCore::VBuffer>(device, "<== AABB - VERTEX BUFFER ==>");
    m_AABBIndexBuffer = std::make_unique<VulkanCore::VBuffer>(device, "<== AABB - INDEX BUFFER ==>");

    m_indexBuffer->MakeIndexBuffer(m_indices);
    m_vertexBuffer->MakeVertexBuffer(m_vertices);


    //========================
    // CALCULATE BOUNDING BOX
    //========================
    glm::vec3 maxPos = m_vertices[0].position;
    glm::vec3 minPos = m_vertices[0].position;;

    for (const auto & i : m_vertices)
    {
        minPos = glm::min(minPos, i.position);
        maxPos = glm::max(maxPos, i.position);
    }

    m_bounds.origin = (maxPos + minPos) /2.f;
    m_bounds.extents = (maxPos - minPos) /2.f;
    m_bounds.radius = glm::length(m_bounds.extents);

    m_AABBVertices = {
        {m_bounds.origin + glm::vec3(-m_bounds.extents.x, -m_bounds.extents.y, -m_bounds.extents.z), {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // V0
        {m_bounds.origin + glm::vec3(+m_bounds.extents.x, -m_bounds.extents.y, -m_bounds.extents.z), {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // V1
        {m_bounds.origin + glm::vec3(+m_bounds.extents.x, +m_bounds.extents.y, -m_bounds.extents.z), {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // V2
        {m_bounds.origin + glm::vec3(-m_bounds.extents.x, +m_bounds.extents.y, -m_bounds.extents.z), {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // V3
        {m_bounds.origin + glm::vec3(-m_bounds.extents.x, -m_bounds.extents.y, +m_bounds.extents.z), {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // V4
        {m_bounds.origin + glm::vec3(+m_bounds.extents.x, -m_bounds.extents.y, +m_bounds.extents.z), {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // V5
        {m_bounds.origin + glm::vec3(+m_bounds.extents.x, +m_bounds.extents.y, +m_bounds.extents.z), {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // V6
        {m_bounds.origin + glm::vec3(-m_bounds.extents.x, +m_bounds.extents.y, +m_bounds.extents.z), {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // V7
    };
    // used for topology line
    m_AABBIndices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    m_AABBVertexBuffer->MakeVertexBuffer(m_AABBVertices);
    m_AABBIndexBuffer->MakeIndexBuffer(m_AABBIndices);
}

void ApplicationCore::VertexArray::Destroy() const {
    m_vertexBuffer->Destroy();
    m_indexBuffer->Destroy();

    m_AABBVertexBuffer->Destroy();
    m_AABBIndexBuffer->Destroy();
}
