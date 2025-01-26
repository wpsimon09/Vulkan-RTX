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


ApplicationCore::StaticMesh::StaticMesh(std::shared_ptr<VulkanStructs::MeshData> geometryData,std::shared_ptr<Material> material, MESH_GEOMETRY_TYPE geometryType)
{
    m_geometryType = geometryType;
    m_meshGeomtryData = geometryData;
    m_transformations = std::make_unique<Transformations>();

    m_material = material;
/**
    m_meshInfo.vertexCount = m_vertexArray->GetVertices().size();
    m_meshInfo.vertexSize = m_vertexArray->GetVertexBuffer().GetBuffeSizeInBytes();
    m_meshInfo.indexCount = m_vertexArray->GetIndices().size();
    m_meshInfo.indexSize = m_vertexArray->GetIndexBuffer().GetBuffeSizeInBytes();
**/

}


const uint32_t ApplicationCore::StaticMesh::GetMeshIndexCount() const {
    return static_cast<uint32_t>(m_meshGeomtryData->indexData.size / sizeof(uint32_t));
}

VulkanStructs::MeshData* ApplicationCore::StaticMesh::GetMeshData()
{
    return m_meshGeomtryData.get();
}

void ApplicationCore::StaticMesh::Update() {
    m_transformations->ComputeModelMatrix();
}

void ApplicationCore::StaticMesh::Destroy()
{
    m_meshGeomtryData.reset();
}

void ApplicationCore::StaticMesh::SetName(std::string name)
{
    m_name = name;
}


std::string ApplicationCore::StaticMesh::MeshGeometryTypeToString(MESH_GEOMETRY_TYPE geometryType) {
    switch (geometryType) {
        case MESH_GEOMETRY_PLANE:
            return "MESH_GEOMETRY_PLANE";
        case MESH_GEOMETRY_CUBE:
            return "MESH_GEOMETRY_CUBE";
        case MESH_GEOMETRY_SPHERE:
            return "MESH_GEOMETRY_SPHERE";
        case MESH_GEOMETRY_POST_PROCESS:
            return "MESH_GEOMETRY_POST_PROCESS";
        case MESH_GEOMETRY_CUSTOM:
            return "MESH_GEOMETRY_CUSTOM";
    }
    return "UNKNOWN";
}
