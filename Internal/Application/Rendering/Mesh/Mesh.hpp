//
// Created by wpsimon09 on 05/10/24.
//

#ifndef MESH_HPP
#define MESH_HPP
#include <memory>

#include "Application/Enums/ClientEnums.hpp"

namespace ApplicationCore
{
    class VertexArray;
    class Mesh {
    public:
        explicit Mesh(MESH_GEOMETRY_TYPE geometryType);

    private:
        void AssignMeshGeometryData();
    private:
        MESH_GEOMETRY_TYPE m_geometryType;
        std::unique_ptr<class ApplicationCore::VertexArray> m_vertexArray;

    };
}


#endif //MESH_HPP
