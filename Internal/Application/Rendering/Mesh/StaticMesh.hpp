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
    class PBRMaterial;
    class SceneNode;
    class Transformations;
    class AssetsManager;
    class VertexArray;



    class StaticMesh{
    public:
        explicit StaticMesh(VulkanStructs::MeshData& geometryData,std::shared_ptr<PBRMaterial> material, EMeshGeometryType geometryType = Custom);
        StaticMesh(const ApplicationCore::StaticMesh& other);

        void ResetMaterial() {m_currentMaterial = m_originalMaterial;};

        void Update();

        void Destroy();

        void SetName(std::string name);

        void SetMaterial(std::shared_ptr<PBRMaterial> material) {m_currentMaterial = material;};
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
        std::shared_ptr<PBRMaterial> GetMaterial() const {return m_currentMaterial;}  ;
        VulkanStructs::MeshData* GetMeshData();

        const std::string& GetName(){return m_name;} const
        void SetName(std::string &newName){m_name = newName;}

        ApplicationCore::Transformations* GetTransformations() const { return m_transformations.get(); }

        MeshInfo& GeteMeshInfo() {return m_meshInfo;}

    private:
        EMeshGeometryType m_geometryType;

        VulkanStructs::MeshData m_meshGeomtryData;
        std::shared_ptr<PBRMaterial> m_currentMaterial;
        std::shared_ptr<PBRMaterial> m_originalMaterial;


        friend class ApplicationCore::SceneNode;
    };
}


#endif //MESH_HPP
