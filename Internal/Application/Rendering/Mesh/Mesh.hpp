//
// Created by wpsimon09 on 05/10/24.
//

#ifndef MESH_HPP
#define MESH_HPP

#include <memory>
#include "Application/Enums/ClientEnums.hpp"

namespace ApplicationCore
{
    class Material;
    class SceneNode;
}

namespace ApplicationCore
{
    class Transformations;
    class AssetsManager;
    class VertexArray;

    class Mesh {
    public:
        explicit Mesh(MESH_GEOMETRY_TYPE geometryType,std::shared_ptr<Material> material, ApplicationCore::AssetsManager& assetsManger);
        const size_t GetMeshVertexArraySize() const;
        const size_t GetMeshIndexArraySize() const;
        const uint32_t GetMeshIndexCount() const;
        const uint32_t GetMeshVertexCount() const;
        ApplicationCore::Transformations* GetTransformations() const { return m_transformations.get(); };
        const VertexArray* GetVertexArray() const {return m_vertexArray;}

        void Update();

        void Destroy();

    private:
        const void AssignMeshGeometryData(ApplicationCore::AssetsManager& assetsManger);
        std::string MeshGeometryTypeToString(MESH_GEOMETRY_TYPE geometryType);
        std::unique_ptr<ApplicationCore::Transformations> m_transformations;
    private:
        MESH_GEOMETRY_TYPE m_geometryType;
        VertexArray* m_vertexArray;
        std::shared_ptr<Material> m_material;

        friend class ApplicationCore::SceneNode;
    };
}


#endif //MESH_HPP
