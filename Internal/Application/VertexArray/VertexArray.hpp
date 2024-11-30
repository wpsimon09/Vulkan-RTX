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


namespace VulkanCore
{
    class VDevice;
    class VBuffer;
}

namespace ApplicationCore
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    class VertexArray {
    public:
        VertexArray(const VulkanCore::VDevice& device,PRIMITIVE_TOPOLOGY topology,const std::vector<Vertex>& vertices,const std::vector<uint32_t>& indices);

        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_indices; };

        const VulkanCore::VBuffer& GetVertexBuffer() const {return *m_vertexBuffer; }
        const VulkanCore::VBuffer& GetIndexBuffer() const {return *m_indexBuffer; }
        void Destroy() const;;

        int GetAttributeCount() {return 3;};

         ~VertexArray() = default;

    private:
        std::unique_ptr<VulkanCore::VBuffer> m_vertexBuffer;
        std::unique_ptr<VulkanCore::VBuffer> m_indexBuffer;
        const std::vector<Vertex> m_vertices;
        const std::vector<uint32_t> m_indices;
        PRIMITIVE_TOPOLOGY m_topology;
    };
}



#endif //VERTEXARRAY_HPP
