//
// Created by wpsimon09 on 05/10/24.

#include "StaticMesh.hpp"
#include "MeshData.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Material/Material.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


ApplicationCore::StaticMesh::StaticMesh(VulkanStructs::MeshData& geometryData,std::shared_ptr<Material> material, EMeshGeometryType geometryType):m_meshGeomtryData(geometryData)
{
    m_geometryType = geometryType;
    m_transformations = std::make_unique<Transformations>();

    m_currentMaterial = material;
    m_originalMaterial = material;
/**
    m_meshInfo.vertexCount = m_vertexArray->GetVertices().size();
    m_meshInfo.vertexSize = m_vertexArray->GetVertexBuffer().GetBuffeSizeInBytes();
    m_meshInfo.indexCount = m_vertexArray->GetIndices().size();
    m_meshInfo.indexSize = m_vertexArray->GetIndexBuffer().GetBuffeSizeInBytes();
**/

}


const uint32_t ApplicationCore::StaticMesh::GetMeshIndexCount() const {
    return static_cast<uint32_t>(m_meshGeomtryData.indexData.size / sizeof(uint32_t));
}

VulkanStructs::MeshData* ApplicationCore::StaticMesh::GetMeshData()
{
    return &m_meshGeomtryData;
}

void ApplicationCore::StaticMesh::Update() {
    m_transformations->ComputeModelMatrix();
}

void ApplicationCore::StaticMesh::Destroy()
{

}

void ApplicationCore::StaticMesh::SetName(std::string name)
{
    m_name = name;
}


std::string ApplicationCore::StaticMesh::MeshGeometryTypeToString(EMeshGeometryType geometryType) {
    switch (geometryType) {
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
