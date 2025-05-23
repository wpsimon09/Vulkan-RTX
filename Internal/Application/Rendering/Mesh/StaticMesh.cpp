//
// Created by wpsimon09 on 05/10/24.

#include "StaticMesh.hpp"
#include "MeshData.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


ApplicationCore::StaticMesh::StaticMesh(VulkanStructs::VMeshData& geometryData, std::shared_ptr<BaseMaterial> material, EMeshGeometryType geometryType)
    : m_meshGeomtryData(geometryData)
{
    m_geometryType    = geometryType;
    m_transformations = std::make_unique<Transformations>();

    m_currentMaterial  = material;
    m_originalMaterial = material;
}

ApplicationCore::StaticMesh::StaticMesh(const ApplicationCore::StaticMesh& other)
    : m_meshGeomtryData(other.m_meshGeomtryData)
{
    m_geometryType    = other.m_geometryType;
    m_transformations = std::make_unique<Transformations>();

    //m_currentMaterial = std::make_shared<PBRMaterial>(*other.m_currentMaterial);
    ///m_originalMaterial = std::make_shared<PBRMaterial>(*other.m_originalMaterial);
}


const uint32_t ApplicationCore::StaticMesh::GetMeshIndexCount() const
{
    return static_cast<uint32_t>(m_meshGeomtryData.indexData.size / sizeof(uint32_t));
}

VulkanStructs::VMeshData* ApplicationCore::StaticMesh::GetMeshData()
{
    return &m_meshGeomtryData;
}

void ApplicationCore::StaticMesh::Update()
{
    m_transformations->ComputeModelMatrix();
}

void ApplicationCore::StaticMesh::Destroy() {}

void ApplicationCore::StaticMesh::SetName(std::string name)
{
    m_name = name;
}


void        ApplicationCore::StaticMesh::SetModelMatrix(glm::mat4& modelMatrix) {
    m_transformations->SetModelMatrix(modelMatrix);
}
std::string ApplicationCore::StaticMesh::MeshGeometryTypeToString(EMeshGeometryType geometryType)
{
    switch(geometryType)
    {
        case Plane:
            return "MESH_GEOMETRY_PLANE";
        case Cube:
            return "MESH_GEOMETRY_CUBE";
        case Sphere:
            return "MESH_GEOMETRY_SPHERE";
        case PostProcessQuad:
            return "MESH_GEOMETRY_POST_PROCESS";
        case Custom:
            return "MESH_GEOMETRY_CUSTOM";
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
