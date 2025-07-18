//
// Created by wpsimon09 on 05/10/24.
//

#ifndef MESH_HPP
#define MESH_HPP

#include <memory>
#include "Application/Enums/ClientEnums.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace ApplicationCore {
class AreaLightNode;
}

namespace ApplicationCore {
class DirectionLightNode;
}

namespace ApplicationCore {
struct Vertex;
class SceneNode;
class Transformations;
class AssetsManager;
class VertexArray;


class StaticMesh
{
  public:
    explicit StaticMesh(VulkanStructs::VMeshData&     geometryData,
                        std::shared_ptr<BaseMaterial> material,
                        EMeshGeometryType             geometryType = Custom);
    StaticMesh(const ApplicationCore::StaticMesh& other);

    void ResetMaterial() { m_currentMaterial = m_originalMaterial; };

    void Update();

    void Destroy();

    void SetName(std::string name);

    void SetMaterial(std::shared_ptr<BaseMaterial> material) { m_currentMaterial = material; };

    void SetModelMatrix(glm::mat4& modelMatrix);

  private:
    std::string                                       MeshGeometryTypeToString(EMeshGeometryType geometryType);
    std::string                                       m_name;
    std::unique_ptr<ApplicationCore::Transformations> m_transformations;

    struct MeshInfo
    {
        int numberOfTriangles = 0;
        int vertexCount       = 0;
        int indexCount        = 0;
        int vertexSize        = 0;
        int indexSize         = 0;

    } m_meshInfo{};

  public:
    const uint32_t                GetMeshIndexCount() const;
    std::shared_ptr<BaseMaterial> GetMaterial() const { return m_currentMaterial; };
    VulkanStructs::VMeshData*     GetMeshData();

    const std::string& GetName() { return m_name; }
    const void         SetName(std::string& newName) { m_name = newName; }

    ApplicationCore::Transformations* GetTransformations() const { return m_transformations.get(); }

    MeshInfo& GeteMeshInfo() { return m_meshInfo; }

  private:
    EMeshGeometryType m_geometryType;

    VulkanStructs::VMeshData      m_meshGeomtryData;
    std::shared_ptr<BaseMaterial> m_currentMaterial;
    std::shared_ptr<BaseMaterial> m_originalMaterial;


    friend class ApplicationCore::SceneNode;
};
}  // namespace ApplicationCore


#endif  //MESH_HPP
