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
}


#endif //MESHDATA_HPP
