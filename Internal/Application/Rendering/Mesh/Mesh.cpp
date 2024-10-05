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
        m_vertexArray = std::make_unique<ApplicationCore::VertexArray>(TOPOLOGY_TRIANGLE_LIST,ApplicationCore::MeshData::planeVertices, ApplicationCore::MeshData::planeIndices) ;
        break;
    };
        default:
            break;
    }
    Utils::Logger::LogInfoVerboseOnly("Assigning vertices of type: " + MeshGeometryTypeToString(m_geometryType));
}

std::string ApplicationCore::Mesh::MeshGeometryTypeToString(MESH_GEOMETRY_TYPE geometryType) {
    switch (geometryType) {
        case MESH_GEOMETRY_PLANE:
            return "MESH_GEOMETRY_PLANE";
        case MESH_GEOMETRY_CUBE:
            return "MESH_GEOMETRY_CUBE";
        case MESH_GEOMETRY_SPHERE:
            return "MESH_GEOMETRY_SPHERE";
        case MESH_GEOMETRY_POST_PROCESS:
            return "MESH_GEOMETRY_POST_PROCESS";
    }
    return "UNKNOWN";
}
