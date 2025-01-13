//
// Created by wpsimon09 on 04/10/24.
//

#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include <memory>
#include <optional>
#include <vector>
#include "Application/Enums/ClientEnums.hpp"
#include "glm/glm.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"



namespace VulkanCore
{
    class VDevice;
    class VBuffer;
}

namespace ApplicationCore
{
    struct Vertex
    {
        glm::vec3 position = {0.0f, 0.0F, 0.0f};
        glm::vec3 normal {0.0f, 1.0f, 0.0f};
        glm::vec2 uv = {0.0f, 0.0f};
    };

    class VertexArray {
    public:
        VertexArray(const VulkanCore::VDevice& device,PRIMITIVE_TOPOLOGY topology,const std::vector<Vertex>& vertices,const std::vector<uint32_t>& indices);
        VertexArray(const VulkanCore::VDevice& device,VulkanStructs::BufferInfo& vertexBuffer,VulkanStructs::BufferInfo& indexBuffer, VulkanStructs::BufferInfo& vertexBufferBB, VulkanStructs::BufferInfo& indexBufferBB);

        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_indices; }
        VulkanStructs::Bounds* GetBounds() {return &m_bounds;}
        uint32_t GetAABBIndexCount() const { return m_AABBIndices.size(); }

        const VulkanCore::VBuffer& GetVertexBuffer() const {return *m_vertexBuffer; }
        const VulkanCore::VBuffer& GetIndexBuffer() const {return *m_indexBuffer; }
        const VulkanCore::VBuffer& GetAABBVertexBuffer() const {return *m_AABBVertexBuffer; }
        const VulkanCore::VBuffer& GetAABBIndexBuffer() const {return *m_AABBIndexBuffer; }
        void Destroy() const;;

        int GetAttributeCount() {return 3;};

         ~VertexArray() = default;

    private:
        // TODO : instead of having this VBuffer unique it can be made a const reference tho the vkBuffer allocated at once

        VulkanStructs::BufferInfo m_vertexBuffer2;
        VulkanStructs::BufferInfo m_indexBuffer2;
        VulkanStructs::BufferInfo m_vertexBufferBB;
        VulkanStructs::BufferInfo m_indexBufferBB;

        std::unique_ptr<VulkanCore::VBuffer> m_vertexBuffer;
        std::unique_ptr<VulkanCore::VBuffer> m_indexBuffer;
        std::unique_ptr<VulkanCore::VBuffer> m_AABBVertexBuffer;
        std::unique_ptr<VulkanCore::VBuffer> m_AABBIndexBuffer;
        const std::vector<Vertex> m_vertices;
        const std::vector<uint32_t> m_indices;

        std::vector<Vertex> m_AABBVertices;
        std::vector<uint32_t> m_AABBIndices;
        VulkanStructs::Bounds m_bounds;

        PRIMITIVE_TOPOLOGY m_topology;
    };
}



#endif //VERTEXARRAY_HPP
