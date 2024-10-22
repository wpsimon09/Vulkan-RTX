//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"

namespace ApplicationCore {
    AssetsManager::AssetsManager(const VulkanCore::VDevice &device):m_device(device) {
        LoadPredefinedMeshes();
    }

    void AssetsManager::DeleteAll() {
        for (auto& meshData: m_meshData) {
            meshData.second->Destroy();
        }
    }

    const VertexArray & AssetsManager::GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType) const {
        auto result = m_meshData.find(geometryType);
        assert(result != m_meshData.end());
        return *result->second;
    }

    void AssetsManager::LoadPredefinedMeshes() {
        m_meshData.insert(std::make_pair(MESH_GEOMETRY_PLANE, std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::planeVertices, MeshData::planeIndices)));
        m_meshData.insert(std::make_pair(MESH_GEOMETRY_CUBE, std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::cubeVertices, MeshData::cubeIndices)));
    }
} // ApplicationCore