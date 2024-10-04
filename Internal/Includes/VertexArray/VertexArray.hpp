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
    struct Vertex
    {
        std::optional<glm::vec3> position;
        std::optional<glm::vec3> normal;
        std::optional<glm::vec2> uv;

        bool isComplete(){return position.has_value() && normal.has_value() && uv.has_value();}
    };

    class VertexArray {
    public:
        VertexArray(std::vector<Vertex> vertices);
    private:

    };
}



#endif //VERTEXARRAY_HPP
