//
// Created by wpsimon09 on 04/10/24.
//

#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include <optional>
#include <vector>

#include "glm/glm.hpp"


namespace ApplicationCore
{
    enum PRIMITIVE_TOPOLOGY
    {
        //points
        TOPOLOGY_POINT_LIST,

        //lines
        TOPOLOGY_LINE_LIST,
        TOPOLOGY_LINE_STRIP,

        //triangles
        TOPOLOGY_TRIANGLE_LIST,
        TOPOLOGY_TRIANGLE_STRIP,
        TOPOLOGY_TRIANGLE_FAN,

        //patch (tesselation only)
        TOPOLOGY_PATCH
 };

    struct Vertex
    {
        std::optional<glm::vec3> position;
        std::optional<glm::vec3> normal;
        std::optional<glm::vec2> uv;

        bool IsComplete(){return position.has_value() && normal.has_value() && uv.has_value();}
    };

    class VertexArray {
    public:
        VertexArray(PRIMITIVE_TOPOLOGY topology,const std::vector<Vertex>& vertices,const std::vector<uint32_t>& indices);

        const std::vector<Vertex>& GetVertices() { return m_vertice; };
        const std::vector<uint32_t>& GetIndices() { return m_indices; };

        int GetAttributeCount() {return 3;};

    private:
        const std::vector<Vertex>& m_vertice;
        const std::vector<uint32_t>& m_indices;
        PRIMITIVE_TOPOLOGY m_topology;

        //----------------------------
        // EQUALITY OPERATORS OVERLOAD
        //----------------------------
        friend bool operator==(const VertexArray &lhs, const VertexArray &rhs) {
            return lhs.m_vertice == rhs.m_vertice
                && lhs.m_indices == rhs.m_indices
                && lhs.m_topology == rhs.m_topology;
        }

        friend bool operator!=(const VertexArray &lhs, const VertexArray &rhs) {
            return !(lhs == rhs);
        }
    };
}



#endif //VERTEXARRAY_HPP
