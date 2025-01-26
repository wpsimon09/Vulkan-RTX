//
// Created by wpsimon09 on 05/10/24.
//

#ifndef MESH_HPP
#define MESH_HPP

#include <memory>
#include "Application/Enums/ClientEnums.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace ApplicationCore
{
    struct Vertex;
    class Material;
    class SceneNode;
    class Transformations;
    class AssetsManager;
    class VertexArray;



    class StaticMesh{
    public:
        explicit StaticMesh(VulkanStructs::MeshData& geometryData,std::shared_ptr<Material> material, EMeshGeometryType geometryType = Custom);

        void Update();

        void Destroy();

        void SetName(std::string name);
    private:
        std::string MeshGeometryTypeToString(EMeshGeometryType geometryType);
        std::string m_name;
        std::unique_ptr<ApplicationCore::Transformations> m_transformations;

        struct MeshInfo
        {
            int numberOfTriangles = 0;
            int vertexCount = 0;
            int indexCount = 0;
            int vertexSize = 0;
            int indexSize = 0;

        }m_meshInfo{};

    public:
        const uint32_t GetMeshIndexCount() const;
        std::shared_ptr<Material> GetMaterial() const {return m_material;}  ;
        VulkanStructs::MeshData* GetMeshData();

        const std::string& GetName(){return m_name;} const
        void SetName(std::string &newName){m_name = newName;}

        VulkanStructs::RenderingMetaData& GetRenderingMetaData()  {return m_renderingMetaData;}
        ApplicationCore::Transformations* GetTransformations() const { return m_transformations.get(); }

        MeshInfo& GeteMeshInfo() {return m_meshInfo;}

    private:
        EMeshGeometryType m_geometryType;

        VulkanStructs::MeshData m_meshGeomtryData;
        std::shared_ptr<Material> m_material;

        VulkanStructs::RenderingMetaData m_renderingMetaData;

        friend class ApplicationCore::SceneNode;
    };
}


#endif //MESH_HPP
