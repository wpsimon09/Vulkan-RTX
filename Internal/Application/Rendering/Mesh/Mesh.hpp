//
// Created by wpsimon09 on 05/10/24.
//

#ifndef MESH_HPP
#define MESH_HPP

#include <memory>
#include "Application/Enums/ClientEnums.hpp"

namespace ApplicationCore
{
    class AssetsManager;
    class VertexArray;
    class Mesh {
    public:
        Mesh(MESH_GEOMETRY_TYPE geometryType, ApplicationCore::AssetsManager& assetsManger);
        const size_t GetMeshVertexArraySize() const;
        const size_t GetMeshIndexArraySize() const;
        const uint32_t GetMeshIndexCount() const;
        const uint32_t GetMeshVertexCount() const;
        const VertexArray* GetVertexArray() const {return m_vertexArray;}
        void Destroy();

    private:
        const void AssignMeshGeometryData(ApplicationCore::AssetsManager& assetsManger);
        std::string MeshGeometryTypeToString(MESH_GEOMETRY_TYPE geometryType);
    private:
        MESH_GEOMETRY_TYPE m_geometryType;
        VertexArray* m_vertexArray;
    };
}


#endif //MESH_HPP
