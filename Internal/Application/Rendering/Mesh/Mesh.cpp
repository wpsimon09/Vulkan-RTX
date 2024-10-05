//
// Created by wpsimon09 on 05/10/24.
//

#include "Mesh.hpp"

#include "MeshData.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/VertexArray/VertexArray.hpp"

ApplicationCore::Mesh::Mesh(MESH_GEOMETRY_TYPE geometryType) {
    m_geometryType = geometryType;
    AssignMeshGeometryData();
}

void ApplicationCore::Mesh::AssignMeshGeometryData() {
    switch (m_geometryType) {
    case MESH_GEOMETRY_PLANE: {
        std::make_unique<VertexArray>(TOPOLOGY_TRIANGLE_LIST,ApplicationCore::MeshData::planeVertices, ApplicationCore::MeshData::planeIndices) ;
        break;
    };
        default:
            break;
    }
    Utils::Logger::LogInfoVerboseOnly("Assigning vertices of type: " + std::to_string(m_geometryType));
}
