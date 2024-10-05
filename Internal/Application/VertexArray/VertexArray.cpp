//
// Created by wpsimon09 on 04/10/24.
//

#include "VertexArray.hpp"

ApplicationCore::VertexArray::VertexArray(PRIMITIVE_TOPOLOGY topology, const std::vector<Vertex> &vertices,
    const std::vector<uint32_t> &indices):m_vertice(vertices), m_indices(indices) {

    m_topology = topology;
}
