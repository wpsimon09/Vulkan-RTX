//
// Created by wpsimon09 on 05/10/24.

#include "Mesh.hpp"
#include "MeshData.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Material/Material.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


ApplicationCore::Mesh::Mesh(std::shared_ptr<VertexArray> geometryData,std::shared_ptr<Material> material, MESH_GEOMETRY_TYPE geometryType)
{
    m_geometryType = geometryType;
    m_vertexArray = geometryData;
    m_transformations = std::make_unique<Transformations>();

    m_material = material;
/**
    m_meshInfo.vertexCount = m_vertexArray->GetVertices().size();
    m_meshInfo.vertexSize = m_vertexArray->GetVertexBuffer().GetBuffeSizeInBytes();
    m_meshInfo.indexCount = m_vertexArray->GetIndices().size();
    m_meshInfo.indexSize = m_vertexArray->GetIndexBuffer().GetBuffeSizeInBytes();
**/

}

const size_t ApplicationCore::Mesh::GetMeshVertexArraySize() const {
    return m_vertexArray->GetVertices().size() * sizeof(Vertex);
}

const size_t ApplicationCore::Mesh::GetMeshIndexArraySize() const {
    return m_vertexArray->GetIndices().size() * sizeof(unsigned int);
}

const uint32_t ApplicationCore::Mesh::GetMeshIndexCount() const {
    return static_cast<uint32_t>(m_vertexArray->GetIndices().size());
}

const uint32_t ApplicationCore::Mesh::GetMeshVertexCount() const {
    return static_cast<uint32_t>(m_vertexArray->GetVertices().size());
}


void ApplicationCore::Mesh::Update() {
    m_transformations->ComputeModelMatrix();
}

void ApplicationCore::Mesh::Destroy()
{

}

void ApplicationCore::Mesh::SetName(std::string name)
{
    m_name = name;
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
        case MESH_GEOMETRY_CUSTOM:
            return "MESH_GEOMETRY_CUSTOM";
    }
    return "UNKNOWN";
}
