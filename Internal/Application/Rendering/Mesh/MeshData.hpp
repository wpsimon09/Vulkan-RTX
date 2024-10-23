//
// Created by wpsimon09 on 04/10/24.
//

#ifndef MESHDATA_HPP
#define MESHDATA_HPP
#include <vector>

#include "Application/VertexArray/VertexArray.hpp"

namespace ApplicationCore::MeshData
{
    const std::vector<ApplicationCore::Vertex> planeVertices = {
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
        {{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // Bottom-right
        {{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Top-right
        {{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Top-left

        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
    };

    const std::vector<uint32_t> planeIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };

    const std::vector<Vertex> cubeVertices = {
    // Front face
    {{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},  // Vertex 0
    {{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},  // Vertex 1
    {{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},  // Vertex 2
    {{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},  // Vertex 3

    // Back face
    {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},  // Vertex 4
    {{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},  // Vertex 5
    {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},  // Vertex 6
    {{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},  // Vertex 7

    // Left face
    {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 8
    {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 9
    {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},  // Vertex 10
    {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 11

    // Right face
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 12
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 13
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 14
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},  // Vertex 15

    // Top face
    {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 16
    {{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 17
    {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},  // Vertex 18
    {{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 19

    // Bottom face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 20
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 21
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 22
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}}   // Vertex 23
};

    const std::vector<uint32_t> cubeIndices = {
        // Front face
        0, 1, 2, 2, 3, 0,

        // Right face
        13, 12, 15, 15, 14, 13,

        // Back face
        6, 5, 4, 4, 7, 6,

        // Left face
        8, 9, 10, 10, 11, 8,

        // Top face
        16, 17, 18, 18, 19, 16,

        // Bottom face
        21, 20, 23, 23, 22, 21
    };

    const std::vector<Vertex> triangleVertices = {
        // First triangle vertex
        {{ 0.0f,  0.5f, 0.0f}, {0.0f,  0.0f, 1.0f}, {0.5f, 1.0f}},  // Top vertex (position, normal, UV)

        // Second triangle vertex
        {{-0.5f, -0.5f, 0.0f}, {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}},  // Bottom-left vertex

        // Third triangle vertex
        {{ 0.5f, -0.5f, 0.0f}, {0.0f,  0.0f, 1.0f}, {1.0f, 0.0f}}   // Bottom-right vertex
    };

    const std::vector<uint32_t> triangleIndices = {
        0, 1, 2  // Indices for the triangle
    };

    static inline void GenerateSphere(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                Vertex tempVertex {};
                tempVertex.position = glm::vec3(xPos, yPos, zPos);
                tempVertex.normal = glm::vec3(xPos, yPos, zPos);
                tempVertex.uv = glm::vec2(xSegment, ySegment);
                vertices.push_back(tempVertex);
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow)
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

    }

    const std::vector<Vertex> crossVertices = {
    // Vertices for the vertical stem of the cross (rectangular prism)
    // Front face (stem)
    {{-0.2f, -1.0f,  0.2f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},  // Vertex 0
    {{ 0.2f, -1.0f,  0.2f}, {0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},  // Vertex 1
    {{ 0.2f,  0.2f,  0.2f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},  // Vertex 2
    {{-0.2f,  0.2f,  0.2f}, {0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},  // Vertex 3

    // Back face (stem)
    {{-0.2f, -1.0f, -0.2f}, {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},  // Vertex 4
    {{ 0.2f, -1.0f, -0.2f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},  // Vertex 5
    {{ 0.2f,  0.2f, -0.2f}, {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},  // Vertex 6
    {{-0.2f,  0.2f, -0.2f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},  // Vertex 7

    // Left face (stem)
    {{-0.2f, -1.0f, -0.2f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 8
    {{-0.2f, -1.0f,  0.2f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 9
    {{-0.2f,  0.2f,  0.2f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},  // Vertex 10
    {{-0.2f,  0.2f, -0.2f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 11

    // Right face (stem)
    {{ 0.2f, -1.0f, -0.2f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 12
    {{ 0.2f, -1.0f,  0.2f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 13
    {{ 0.2f,  0.2f,  0.2f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 14
    {{ 0.2f,  0.2f, -0.2f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},  // Vertex 15

    // Top face (stem)
    {{-0.2f,  0.2f, -0.2f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},   // Vertex 16
    {{-0.2f,  0.2f,  0.2f}, {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},   // Vertex 17
    {{ 0.2f,  0.2f,  0.2f}, {0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},   // Vertex 18
    {{ 0.2f,  0.2f, -0.2f}, {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},   // Vertex 19

    // Bottom face (stem)
    {{-0.2f, -1.0f, -0.2f}, {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},  // Vertex 20
    {{-0.2f, -1.0f,  0.2f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},  // Vertex 21
    {{ 0.2f, -1.0f,  0.2f}, {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},  // Vertex 22
    {{ 0.2f, -1.0f, -0.2f}, {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},  // Vertex 23

    // Horizontal arms of the cross (rectangular prism)
    // Front face (arms)
    {{-0.8f,  0.2f,  0.2f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},  // Vertex 24
    {{ 0.8f,  0.2f,  0.2f}, {0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},  // Vertex 25
    {{ 0.8f,  0.5f,  0.2f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},  // Vertex 26
    {{-0.8f,  0.5f,  0.2f}, {0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},  // Vertex 27

    // Back face (arms)
    {{-0.8f,  0.2f, -0.2f}, {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},  // Vertex 28
    {{ 0.8f,  0.2f, -0.2f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},  // Vertex 29
    {{ 0.8f,  0.5f, -0.2f}, {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},  // Vertex 30
    {{-0.8f,  0.5f, -0.2f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}}   // Vertex 31
    };

    const std::vector<uint32_t> crossIndices = {
        // Stem indices
        0, 1, 2, 2, 3, 0,  // Front face
        5, 4, 7, 7, 6, 5,  // Back face
        8, 9, 10, 10, 11, 8,  // Left face
        13, 12, 15, 15, 14, 13,  // Right face
        16, 17, 18, 18, 19, 16,  // Top face
        21, 20, 23, 23, 22, 21,  // Bottom face

        // Arms indices
        24, 25, 26, 26, 27, 24,  // Front face
        29, 28, 31, 31, 30, 29,  // Back face
        24, 28, 31, 31, 27, 24,  // Left face (arms)
        25, 29, 30, 30, 26, 25   // Right face (arms)
    };



}


#endif //MESHDATA_HPP
