//
// Created by wpsimon09 on 04/10/24.
//

#ifndef MESHDATA_HPP
#define MESHDATA_HPP
#include <vector>

#include "Application/VertexArray/VertexArray.hpp"

namespace ApplicationCore::MeshData
{
    inline std::vector<ApplicationCore::Vertex> planeVertices = {
        {{-0.5f, -0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom-left
        {{ 0.5f, -0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Bottom-right
        {{ 0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top-right
        {{-0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}  // Top-left
    };
    
    inline std::vector<uint32_t> planeIndices = {
        0, 1, 2,  // First triangle (Top-left, Top-right, Bottom-left) - CCW
        0, 2, 3   // Second triangle (Top-right, Bottom-right, Bottom-left) - CCW
    };


    inline std::vector<Vertex> cubeVertices = {
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

    inline std::vector<uint32_t> cubeIndices = {
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

    inline std::vector<Vertex> triangleVertices = {
        // First triangle vertex
        {{ 0.0f,  0.5f, 0.0f}, {0.0f,  0.0f, 1.0f}, {0.5f, 1.0f}},  // Top vertex (position, normal, UV)

        // Second triangle vertex
        {{-0.5f, -0.5f, 0.0f}, {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}},  // Bottom-left vertex

        // Third triangle vertex
        {{ 0.5f, -0.5f, 0.0f}, {0.0f,  0.0f, 1.0f}, {1.0f, 0.0f}}   // Bottom-right vertex
    };

    inline std::vector<uint32_t> triangleIndices = {
        0, 1, 2  // Indices for the triangle
    };

    static inline void GenerateSphere(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;

        // Generate vertices
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
                tempVertex.normal = glm::vec3(xPos, yPos, zPos); // Normals are the same as the position for a unit sphere
                tempVertex.uv = glm::vec2(xSegment, ySegment); // Texture coordinates
                vertices.push_back(tempVertex);
            }
        }

        // Generate indices for triangle list
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x < X_SEGMENTS; ++x)
            {
                // Calculate the indices for the four vertices of the current quad
                unsigned int topLeft = y * (X_SEGMENTS + 1) + x;
                unsigned int topRight = y * (X_SEGMENTS + 1) + (x + 1);
                unsigned int bottomLeft = (y + 1) * (X_SEGMENTS + 1) + x;
                unsigned int bottomRight = (y + 1) * (X_SEGMENTS + 1) + (x + 1);

                // Add two triangles for the current quad
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

    }

    inline std::vector<Vertex> crossVertices = {
        // Vertical bar (shifted down)
        {{-0.1f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},  // Bottom-left
        {{ 0.1f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Bottom-right
        {{ 0.1f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  // Top-right
        {{-0.1f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // Top-left

        // Horizontal bar (higher up on Y-axis)
        {{-0.4f,  0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},  // Left of horizontal bar
        {{ 0.4f,  0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Right of horizontal bar
        {{ 0.4f,  0.2f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  // Top-right corner
        {{-0.4f,  0.2f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}   // Top-left corner
    };

    inline std::vector<uint32_t> crossIndices = {
        // Vertical bar
        0, 1, 2, 2, 3, 0,

        // Horizontal bar
        4, 5, 6, 6, 7, 4
    };

    inline std::vector<Vertex> fullscreenQuadVertices = {
        // Fullscreen quad vertices
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Bottom-left
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Bottom-right
        {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Top-right
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}  // Top-left
    };

    inline std::vector<uint32_t> fullscreenQuadIndices = {
        // Fullscreen quad indices
        0, 1, 2, 2, 3, 0
    };

    // used for topology line
    inline std::vector<uint32_t> Indices_BB = {
        // Bottom face
        0, 1, 2, 0, 2, 3,
        // Top face
        4, 5, 6, 4, 6, 7,
        // Front face
        0, 1, 5, 0, 5, 4,
        // Back face
        3, 2, 6, 3, 6, 7,
        // Left face
        0, 4, 7, 0, 7, 3,
        // Right face
        1, 5, 6, 1, 6, 2
    };



}


#endif //MESHDATA_HPP
