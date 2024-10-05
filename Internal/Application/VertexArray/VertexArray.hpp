//
// Created by wpsimon09 on 04/10/24.
//

#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include <optional>
#include <vector>
#include "Application/Enums/ClientEnums.hpp"
#include "glm/glm.hpp"


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
        VertexArray(PRIMITIVE_TOPOLOGY topology,const std::vector<Vertex>& vertices,const std::vector<uint32_t>& indices);

        const std::vector<Vertex>& GetVertices() { return m_vertice; };
        const std::vector<uint32_t>& GetIndices() { return m_indices; };

        int GetAttributeCount() {return 3;};

         ~VertexArray() = default;

    private:
        const std::vector<Vertex>& m_vertice;
        const std::vector<uint32_t>& m_indices;
        PRIMITIVE_TOPOLOGY m_topology;
    };
}



#endif //VERTEXARRAY_HPP
